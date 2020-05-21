#!/bin/bash -xe

readonly DIR=$( cd $(dirname $0)/.. ; pwd -P )
DOCKER_IMAGE="${DOCKER_IMAGE:?DOCKER_IMAGE variable is not defined}"
CI_ENV=$(env | awk -F= -v key='^(BRANCH_|BUILD_|HUDSON_|JENKINS_|JOB_|CHANGE_|TRAVIS_)' '$1 ~ key {print "-e " $1}')
docker run -i --rm \
   --cap-add SYS_PTRACE \
   -v /tmp/cache/hunter:/root/.hunter \
   -v ${DIR}:/workdir \
   -w /workdir \
   -e CTEST_OUTPUT_ON_FAILURE \
   -e GITHUB_HUNTER_USERNAME \
   -e GITHUB_HUNTER_TOKEN \
   -e GIT_HUB_REPOSITORY \
   -e GIT_HUB_TOKEN \
   -e GIT_HUB_USERNAME \
   -e CODECOV_TOKEN \
   -e SONAR_TOKEN \
    $CI_ENV \
   "${DOCKER_IMAGE}" $@
