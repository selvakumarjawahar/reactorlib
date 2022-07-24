#!/bin/bash

# Constants
DOCKER_IMAGE_NAME="ubuntu_jammy_develop"
DOCKER_URL="selvadockers/${DOCKER_IMAGE_NAME}"
BUILD_DIR="build"
RED='\033[0;31m'
GREEN='\033[0;32m'
BLUE='\033[0;34m'
NC='\033[0m'

# Global Settings
set -e

show_help()
{
    cat << EOF
    Options
    -r|--build-release     : Does a clean build, runs tests and creates release package. Use this option to prepare a release package.
    -d|--build-docker      : Builds all required Docker images
    -e|--push-docker       : Builds all required Docker images and pushes to docker hub
    -c|--compile           : Compiles the code in Debug mode. Use this option for development
    -u|--unit-test         : Runs unit tests
    -s|--static-analysis   : Runs static analysis
    -f|--format            : Uses clang format to format all the files
    -h|--help              : Shows this message
EOF
}

log()
{
    case ${1} in
        [ERROR]* ) echo -e ${RED}${1}${NC};;
        [INFO]*  ) echo -e ${GREEN}${1}${NC};;
        [WARN]*  ) echo -e ${BLUE}${1}${NC};;
        *        ) echo -e ${GREEN}${1}${NC};;
    esac
}

die_with_error()
{
    log "[ERROR]${1}"
    exit 1
}

check_dir()
{
    [ ! -d ${1} ] && die_with_error "directory ${1} does not exist" || :
}

check_file()
{
    [ ! -f ${1} ] && die_with_error "file ${1} does not exist" || :
}

check_executable()
{
    command -v ${1} >/dev/null 2>&1 || die_with_error "executable ${1} does not exist"
}

check_env()
{
    check_executable docker
}

build_docker()
{
    log "[INFO] Build docker"
    check_dir docker
    cd docker
    check_file Dockerfile
    log "[INFO] Going for docker build"
    docker build -t ${DOCKER_URL} .
    log "[INFO] Docker Build Successfully"
    cd -
}

push_docker()
{
    log "[INFO] Pushing docker image to docker hub"
    docker image inspect ${DOCKER_URL} >/dev/null 2>&1 && docker push ${DOCKER_URL} || die_with_error "Docker image ${DOCKER_URL} does not exists"
    log "[INFO] Docker pushed successfully"
}

compile_debug()
{
    mkdir -p build
    docker run --rm -v "$(pwd):/share" -u $(id -u):$(id -g) -w "/share" ${DOCKER_URL} /bin/bash -c "cd build && cmake .. &&  cmake --build . --config Debug"
}

compile_release()
{
    mkdir -p build
    docker run --rm -v "$(pwd):/share" -u $(id -u):$(id -g) -w "/share" ${DOCKER_URL} /bin/bash -c "cd build && cmake .. &&  cmake --build . --config Release"
}

run_clang_format()
{
    log "[INFO] Running clang format"
    mkdir -p build
    docker run --rm -v "$(pwd):/share" -u $(id -u):$(id -g) -w "/share" ${DOCKER_URL} /bin/bash -c "cd build && cmake .. &&  cmake --build . --target clangformat"
}

run_unit_tests()
{
    log "[INFO] Running Unit Tests"
    # clean
    mkdir -p build
    compile_debug
    docker run --rm -v "$(pwd):/share" -u $(id -u):$(id -g) -w "/share" ${DOCKER_URL} /bin/bash -c "cd build &&  ctest --verbose"
}

run_static_analysis()
{
    log "[INFO] Running Static Analysis"
    clean
    mkdir -p build
    compile_debug
    docker run --rm -v "$(pwd):/share" -u $(id -u):$(id -g) -w "/share" ${DOCKER_URL} /bin/bash -c "cd build &&  cmake --build . --target clang-tidy-check"
}

clean()
{
    log "[INFO] Cleaning build"
    rm -fr build
}

build_release()
{
    clean
    compile_release
    run_unit_tests
    run_static_analysis
}

main()
{
    if [[ $# -eq 0 ]] ; then
        show_help
        exit 1
    fi
    check_env

    while [[ $# -gt 0 ]]; do
        case "$1" in
            -r|--build-release)
                build_release
                exit 0
                ;;
            -d|--build-docker)
                build_docker
                exit 0
                ;;
            -e|--push-docker)
                build_docker
                push_docker
                exit 0
                ;;
           -c|--compile)
                clean
                compile_debug
                exit 0
                ;;
            -u|--unit-test)
                run_unit_tests
                exit 0
                ;;
            -s|--static-analysis)
                run_static_analysis
                exit 0
                ;;
            -f|--format)
                shift
                run_clang_format
                exit 0
                ;;
            -h|--help)
                show_help
                exit 0
                ;;
            *)
                echo "Unknown Command"
                show_help
                exit 1
                ;;
        esac
    done
}

main $@
