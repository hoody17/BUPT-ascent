#!/bin/bash
set -e
#
# run at root of ascent repo
#
date
# run spack install, this will install ascent@develop
export ASCENT_VERSION=0.8.0
export BASE_DIR=$PSCRATCH/ASCENT_INSTALL 
#/project/projectdirs/alpine/software/ascent/
export DEST_DIR=$BASE_DIR/${ASCENT_VERSION}/
mkdir -p $DEST_DIR
# ^vtk-h~blt_find_mpi
python3 scripts/uberenv/uberenv.py --spec="%gcc~blt_find_mpi ^conduit~blt_find_mpi ^vtk-h~blt_find_mpi ^hdf5~mpi ^cmake~openssl~ncurses" \
       --pull \
       --install \
       --spack-config-dir="scripts/uberenv_configs/spack_configs/configs/nersc/perlmutter_gcc_10.3.0_cuda_11.4/" \
       --prefix=${DEST_DIR}

# gen symlinks to important deps
python3 scripts/spack_install/gen_extra_install_symlinks.py ${DEST_DIR} cmake conduit
# gen env helper script
rm -f public_env.sh
python3 scripts/spack_install/gen_public_install_env_script.py ${DEST_DIR} PrgEnv-gnu cudatoolkit/21.9_11.4  cpe-cuda/21.12
chmod a+x public_env.sh
cp public_env.sh $BASE_DIR/${ASCENT_VERSION}/ascent_permutter_setup_env_gcc_cuda.sh
# change perms to group write
chgrp -R csc340 $BASE_DIR/${ASCENT_VERSION}
chmod g+rwX -R $BASE_DIR/${ASCENT_VERSION}
# world shared no longer means world shared by default, so lets change perms for all
chmod a+rX -R ${BASE_DIR}/${ASCENT_VERSION}/
date
