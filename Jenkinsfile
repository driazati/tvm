#!groovy
// -*- mode: groovy -*-

// Licensed to the Apache Software Foundation (ASF) under one
// or more contributor license agreements.  See the NOTICE file
// distributed with this work for additional information
// regarding copyright ownership.  The ASF licenses this file
// to you under the Apache License, Version 2.0 (the
// "License"); you may not use this file except in compliance
// with the License.  You may obtain a copy of the License at
//
//   http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing,
// software distributed under the License is distributed on an
// "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
// KIND, either express or implied.  See the License for the
// specific language governing permissions and limitations
// under the License.

// Jenkins pipeline
// See documents at https://jenkins.io/doc/book/pipeline/jenkinsfile/

// Docker env used for testing
// Different image may have different version tag
// because some of them are more stable than anoter.
//
// Docker images are maintained by PMC, cached in dockerhub
// and remains relatively stable over the time.
// Flow for upgrading docker env(need commiter)
//
// - Send PR to upgrade build script in the repo
// - Build the new docker image
// - Tag the docker image with a new version and push to a binary cache.
// - Update the version in the Jenkinsfile, send a PR
// - Fix any issues wrt to the new image version in the PR
// - Merge the PR and now we are in new version
// - Tag the new version as the lates
// - Periodically cleanup the old versions on local workers
//

// ============================= IMPORTANT NOTE =============================
// This file is generated by 'jenkins/generate.py'. Do not edit this file directly!
// Make edits to 'jenkins/Jenkinsfile.j2' and regenerate this with
// 'python3 jenkins/generate.py'

import org.jenkinsci.plugins.pipeline.modeldefinition.Utils
// NOTE: these lines are scanned by docker/dev_common.sh. Please update the regex as needed. -->
ci_lint = 'tlcpack/ci-lint:v0.69'
ci_gpu = 'tlcpack/ci-gpu:v0.82'
ci_cpu = 'tlcpack/ci-cpu:v0.82'
ci_wasm = 'tlcpack/ci-wasm:v0.72'
ci_i386 = 'tlcpack/ci-i386:v0.75'
ci_qemu = 'tlcpack/ci-qemu:v0.12'
ci_arm = 'tlcpack/ci-arm:v0.08'
ci_hexagon = 'tlcpack/ci-hexagon:v0.02'
// <--- End of regex-scanned config.

// Parameters to allow overriding (in Jenkins UI), the images
// to be used by a given build. When provided, they take precedence
// over default values above.
properties([
  parameters([
    string(name: 'ci_arm_param', defaultValue: ''),
    string(name: 'ci_cpu_param', defaultValue: ''),
    string(name: 'ci_gpu_param', defaultValue: ''),
    string(name: 'ci_hexagon_param', defaultValue: ''),
    string(name: 'ci_i386_param', defaultValue: ''),
    string(name: 'ci_lint_param', defaultValue: ''),
    string(name: 'ci_qemu_param', defaultValue: ''),
    string(name: 'ci_wasm_param', defaultValue: ''),
  ])
])

// tvm libraries
tvm_runtime = 'build/libtvm_runtime.so, build/config.cmake'
tvm_lib = 'build/libtvm.so, ' + tvm_runtime
// LLVM upstream lib
tvm_multilib = 'build/libtvm.so, ' +
               'build/libvta_fsim.so, ' +
               tvm_runtime

tvm_multilib_tsim = 'build/libvta_tsim.so, ' +
               tvm_multilib

// command to start a docker container
docker_build = 'docker/build.sh'
// timeout in minutes
max_time = 240
rebuild_docker_images = false

def per_exec_ws(folder) {
  return "workspace/exec_${env.EXECUTOR_NUMBER}/" + folder
}

// initialize source codes
def init_git() {
  checkout scm
  // Add more info about job node
  sh (
    script: './tests/scripts/task_show_node_info.sh',
    label: 'Show executor node info',
  )
  retry(5) {
    timeout(time: 2, unit: 'MINUTES') {
      sh (script: 'git submodule update --init -f', label: 'Update git submodules')
    }
  }
}

def run_in_ecr(task) {
  try {
    // Use a credential so Jenkins knows to scrub the AWS account ID which is nice
    // (but so we don't have to rely it being hardcoded in Jenkins)
    withCredentials([string(
      credentialsId: 'aws-account-id',
      variable: '_ACCOUNT_ID_DO_NOT_USE',
      )]) {
      withEnv([
        "AWS_ACCOUNT_ID=${aws_account_id}",
        "AWS_ECR_URL=${aws_account_id}.dkr.ecr.us-west-2.amazonaws.com",
        'AWS_DEFAULT_REGION=us-west-2']) {
        sh(
          script: '''
            set -x
            aws ecr get-login-password --region $AWS_DEFAULT_REGION | docker login --username AWS --password-stdin $AWS_ECR_URL
          ''',
          label: 'Log in to ECR'
        )
        task()
      }
    }
  } finally {
    sh(
      script: 'rm -f ~/.docker/config.json',
      label: 'Clean up login credentials'
    )
  }
}

def build_image(image_name) {
  hash = sh(
    returnStdout: true,
    script: 'git log -1 --format=\'%h\''
  ).trim()
  def full_name = "${image_name}:${env.BRANCH_NAME}-${hash}"
  sh(
    script: "${docker_build} ${image_name} --spec ${full_name}",
    label: 'Build docker image'
  )
  aws_account_id = sh(
    returnStdout: true,
    script: 'aws sts get-caller-identity | grep Account | cut -f4 -d\\"',
    label: 'Get AWS ID'
  ).trim()

  run_in_ecr({
    sh(
      script: """
        set -x
        docker tag ${full_name} \$AWS_ECR_URL/${full_name}
        docker push \$AWS_ECR_URL/${full_name}
      """,
      label: 'Upload image to ECR'
    )
  })

  sh(
    script: "docker rmi ${full_name}",
    label: 'Remove docker image'
  )

  return full_name
}

def docker_run(image, script, label) {
  exec = {
    withEnv(["IMAGE=${image}"]) {
      sh (
        script: "docker/bash.sh \$IMAGE ${script}",
        label: label,
      )
    }
  }

  if (image.contains("amazon")) {
    run_in_ecr({
      exec()
    })
  } else {
    exec()
  }
}

def should_skip_slow_tests(pr_number) {
  withCredentials([string(
    credentialsId: 'tvm-bot-jenkins-reader',
    variable: 'GITHUB_TOKEN',
  )]) {
    // Exit code of 1 means run slow tests, exit code of 0 means skip slow tests
    result = sh (
      returnStatus: true,
      script: "./tests/scripts/should_run_slow_tests.py --pr '${pr_number}'",
      label: 'Check if CI should run slow tests',
    )
  }
  return result == 0
}

def cancel_previous_build() {
  // cancel previous build if it is not on main.
  if (env.BRANCH_NAME != 'main') {
    def buildNumber = env.BUILD_NUMBER as int
    // Milestone API allows us to cancel previous build
    // with the same milestone number
    if (buildNumber > 1) milestone(buildNumber - 1)
    milestone(buildNumber)
  }
}

def should_skip_ci(pr_number) {
  if (env.BRANCH_NAME == null || !env.BRANCH_NAME.startsWith('PR-')) {
    // never skip CI on build sourced from a branch
    return false
  }
  glob_skip_ci_code = sh (
    returnStatus: true,
    script: "./tests/scripts/git_skip_ci_globs.py",
    label: 'Check if CI should be skipped due to changed files',
  )
  if (glob_skip_ci_code == 0) {
    return true
  }
  withCredentials([string(
    credentialsId: 'tvm-bot-jenkins-reader',
    variable: 'TOKEN',
    )]) {
    // Exit code of 1 means run full CI (or the script had an error, so run
    // full CI just in case). Exit code of 0 means skip CI.
    git_skip_ci_code = sh (
      returnStatus: true,
      script: "./tests/scripts/git_skip_ci.py --pr '${pr_number}'",
      label: 'Check if CI should be skipped',
    )
  }
  return git_skip_ci_code == 0
}

// skips builds from branch indexing; sourced from https://www.jvt.me/posts/2020/02/23/jenkins-multibranch-skip-branch-index/
// execute this before anything else, including requesting any time on an agent
if (currentBuild.getBuildCauses().toString().contains('BranchIndexingCause')) {
  print "INFO: Build skipped due to trigger being Branch Indexing"
  currentBuild.result = 'ABORTED' // optional, gives a better hint to the user that it's been skipped, rather than the default which shows it's successful
  return
}

cancel_previous_build()

stage('Prepare') {
  node('CPU') {
    // When something is provided in ci_*_param, use it, otherwise default with ci_*
    ci_lint = params.ci_lint_param ?: ci_lint
    ci_cpu = params.ci_cpu_param ?: ci_cpu
    ci_gpu = params.ci_gpu_param ?: ci_gpu
    ci_wasm = params.ci_wasm_param ?: ci_wasm
    ci_i386 = params.ci_i386_param ?: ci_i386
    ci_qemu = params.ci_qemu_param ?: ci_qemu
    ci_arm = params.ci_arm_param ?: ci_arm

    sh (script: """
      echo "Docker images being used in this build:"
      echo " ci_lint = ${ci_lint}"
      echo " ci_cpu  = ${ci_cpu}"
      echo " ci_gpu  = ${ci_gpu}"
      echo " ci_wasm = ${ci_wasm}"
      echo " ci_i386 = ${ci_i386}"
      echo " ci_qemu = ${ci_qemu}"
      echo " ci_arm  = ${ci_arm}"
    """, label: 'Docker image names')
  }
}

stage('Sanity Check') {
  timeout(time: max_time, unit: 'MINUTES') {
    node('CPU') {
      ws("workspace/exec_${env.EXECUTOR_NUMBER}/tvm/sanity") {
        init_git()
        is_docs_only_build = sh (
          returnStatus: true,
          script: './tests/scripts/git_change_docs.sh',
          label: 'Check for docs only changes',
        )
        skip_ci = should_skip_ci(env.CHANGE_ID)
        skip_slow_tests = should_skip_slow_tests(env.CHANGE_ID)
        rebuild_docker_images = sh (
          returnStatus: true,
          script: './tests/scripts/git_change_docker.sh',
          label: 'Check for any docker changes',
        )
        if (rebuild_docker_images) {
          // Exit before linting so we can use the newly created Docker images
          // to run the lint
          return
        }
        docker_run(ci_lint, './tests/scripts/task_lint.sh', 'Run lint')
      }
    }
  }
}

if (rebuild_docker_images) {
  stage('Docker Image Build') {
    // TODO in a follow up PR: Find ecr tag and use in subsequent builds
    parallel(
      'ci_arm': {
        node('CPU') {
          timeout(time: max_time, unit: 'MINUTES') {
            init_git()
            new_ci_arm = build_image('ci_arm')
            run_in_ecr({
              ci_arm = "${env.AWS_ECR_URL}/${new_ci_arm}"
            })
          }
        }
      },
      'ci_cpu': {
        node('CPU') {
          timeout(time: max_time, unit: 'MINUTES') {
            init_git()
            new_ci_cpu = build_image('ci_cpu')
            run_in_ecr({
              ci_cpu = "${env.AWS_ECR_URL}/${new_ci_cpu}"
            })
          }
        }
      },
      'ci_gpu': {
        node('CPU') {
          timeout(time: max_time, unit: 'MINUTES') {
            init_git()
            new_ci_gpu = build_image('ci_gpu')
            run_in_ecr({
              ci_gpu = "${env.AWS_ECR_URL}/${new_ci_gpu}"
            })
          }
        }
      },
      'ci_hexagon': {
        node('CPU') {
          timeout(time: max_time, unit: 'MINUTES') {
            init_git()
            new_ci_hexagon = build_image('ci_hexagon')
            run_in_ecr({
              ci_hexagon = "${env.AWS_ECR_URL}/${new_ci_hexagon}"
            })
          }
        }
      },
      'ci_i386': {
        node('CPU') {
          timeout(time: max_time, unit: 'MINUTES') {
            init_git()
            new_ci_i386 = build_image('ci_i386')
            run_in_ecr({
              ci_i386 = "${env.AWS_ECR_URL}/${new_ci_i386}"
            })
          }
        }
      },
      'ci_lint': {
        node('CPU') {
          timeout(time: max_time, unit: 'MINUTES') {
            init_git()
            new_ci_lint = build_image('ci_lint')
            run_in_ecr({
              ci_lint = "${env.AWS_ECR_URL}/${new_ci_lint}"
            })
          }
        }
      },
      'ci_qemu': {
        node('CPU') {
          timeout(time: max_time, unit: 'MINUTES') {
            init_git()
            new_ci_qemu = build_image('ci_qemu')
            run_in_ecr({
              ci_qemu = "${env.AWS_ECR_URL}/${new_ci_qemu}"
            })
          }
        }
      },
      'ci_wasm': {
        node('CPU') {
          timeout(time: max_time, unit: 'MINUTES') {
            init_git()
            new_ci_wasm = build_image('ci_wasm')
            run_in_ecr({
              ci_wasm = "${env.AWS_ECR_URL}/${new_ci_wasm}"
            })
          }
        }
      },
    )
  }

  // If the docker images changed, we need to run the image build before the lint
  // can run since it requires a base docker image. Most of the time the images
  // aren't build though so it's faster to use the same node that checks for
  // docker changes to run the lint in the usual case.
  stage('Sanity Check (re-run)') {
    timeout(time: max_time, unit: 'MINUTES') {
      node('CPU') {
        ws("workspace/exec_${env.EXECUTOR_NUMBER}/tvm/sanity-rerun") {
          init_git()
          docker_run(ci_lint, "./tests/scripts/task_lint.sh", "Run lint")
        }
      }
    }
  }
}

// Run make. First try to do an incremental make from a previous workspace in hope to
// accelerate the compilation. If something is wrong, clean the workspace and then
// build from scratch.
def make(docker_type, path, make_flag) {
  timeout(time: max_time, unit: 'MINUTES') {
    try {
      cmake_build(docker_type, path, make_flag)
      // always run cpp test when build
    } catch (hudson.AbortException ae) {
      // script exited due to user abort, directly throw instead of retry
      if (ae.getMessage().contains('script returned exit code 143')) {
        throw ae
      }
      echo 'Incremental compilation failed. Fall back to build from scratch'
      docker_run(docker_type, "./tests/scripts/task_clean.sh ${path}", 'Clear old cmake workspace')
      cmake_build(docker_type, path, make_flag)
    }
  }
}

// pack libraries for later use
def pack_lib(name, libs) {
  sh (script: """
     echo "Packing ${libs} into ${name}"
     echo ${libs} | sed -e 's/,/ /g' | xargs md5sum
     """, label: 'Stash libraries and show md5')
  stash includes: libs, name: name
}

// unpack libraries saved before
def unpack_lib(name, libs) {
  unstash name
  sh (script: """
     echo "Unpacked ${libs} from ${name}"
     echo ${libs} | sed -e 's/,/ /g' | xargs md5sum
     """, label: 'Unstash libraries and show md5')
}

def ci_setup(image) {
  docker_run(image, './tests/scripts/task_ci_setup.sh', 'Set up CI environment')
}

def python_unittest(image) {
  docker_run(image, './tests/scripts/task_python_unittest.sh', 'Run Python unit tests')
}

def fsim_test(image) {
  docker_run(image, './tests/scripts/task_python_vta_fsim.sh', 'Run VTA tests in FSIM')
}

def cmake_build(image, path, make_flag) {
  docker_run(image, './tests/scripts/task_build.py --sccache-bucket tvm-sccache-prod', 'Run cmake build')
}

def cpp_unittest(image) {
  docker_run("--env CI_NUM_EXECUTORS ${image}", './tests/scripts/task_cpp_unittest.sh', 'Build and run C++ tests')
}

stage('Build') {
  environment {
    SKIP_SLOW_TESTS = "${skip_slow_tests}"
  }
  parallel 'BUILD: GPU': {
    if (!skip_ci) {
      node('CPU') {
        ws("workspace/exec_${env.EXECUTOR_NUMBER}/tvm/build-gpu") {
          init_git()
          docker_run("--no-gpu ${ci_gpu}", './tests/scripts/task_config_build_gpu.sh build', 'Configure GPU build')
          make("${ci_gpu} --no-gpu", 'build', '-j2')
          pack_lib('gpu', tvm_multilib)
          // compiler test
          docker_run("--no-gpu ${ci_gpu}", './tests/scripts/task_config_build_gpu_other.sh build2', 'Configure GPU other build')
          make("${ci_gpu} --no-gpu", 'build2', '-j2')
          pack_lib('gpu2', tvm_multilib)
        }
      }
    }
  },
  'BUILD: CPU': {
    if (!skip_ci && is_docs_only_build != 1) {
      node('CPU') {
        ws("workspace/exec_${env.EXECUTOR_NUMBER}/tvm/build-cpu") {
          init_git()
          docker_run(ci_cpu, './tests/scripts/task_config_build_cpu.sh build', 'Create CPU cmake config')
          make(ci_cpu, 'build', '-j2')
          pack_lib('cpu', tvm_multilib_tsim)
          timeout(time: max_time, unit: 'MINUTES') {
            ci_setup(ci_cpu)
            // sh "${docker_run} ${ci_cpu} ./tests/scripts/task_golang.sh"
            // TODO(@jroesch): need to resolve CI issue will turn back on in follow up patch
            docker_run(ci_cpu, './tests/scripts/task_rust.sh', 'Rust build and test')
          }
        }
      }
    } else {
      Utils.markStageSkippedForConditional('BUILD: CPU')
    }
  },
  'BUILD: WASM': {
    if (!skip_ci && is_docs_only_build != 1) {
      node('CPU') {
        ws("workspace/exec_${env.EXECUTOR_NUMBER}/tvm/build-wasm") {
          init_git()
          docker_run(ci_wasm, './tests/scripts/task_config_build_wasm.sh build', 'Create WASM cmake config')
          make(ci_wasm, 'build', '-j2')
          cpp_unittest(ci_wasm)
          timeout(time: max_time, unit: 'MINUTES') {
            ci_setup(ci_wasm)
            docker_run(ci_wasm, './tests/scripts/task_web_wasm.sh', 'Run WASM lint and tests')
          }
        }
      }
    } else {
      Utils.markStageSkippedForConditional('BUILD: WASM')
    }
  },
  'BUILD: i386': {
    if (!skip_ci && is_docs_only_build != 1) {
      node('CPU') {
        ws("workspace/exec_${env.EXECUTOR_NUMBER}/tvm/build-i386") {
          init_git()
          docker_run(ci_i386, './tests/scripts/task_config_build_i386.sh build', 'Create i386 cmake config')
          make(ci_i386, 'build', '-j2')
          pack_lib('i386', tvm_multilib_tsim)
        }
      }
    } else {
      Utils.markStageSkippedForConditional('BUILD: i386')
    }
  },
  'BUILD: arm': {
    if (!skip_ci && is_docs_only_build != 1) {
      node('ARM') {
        ws("workspace/exec_${env.EXECUTOR_NUMBER}/tvm/build-arm") {
          init_git()
          docker_run(ci_arm, './tests/scripts/task_config_build_arm.sh build', 'Create ARM cmake config')
          make(ci_arm, 'build', '-j4')
          pack_lib('arm', tvm_multilib)
        }
      }
     } else {
      Utils.markStageSkippedForConditional('BUILD: arm')
    }
  },
  'BUILD: QEMU': {
    if (!skip_ci && is_docs_only_build != 1) {
      node('CPU') {
        ws("workspace/exec_${env.EXECUTOR_NUMBER}/tvm/build-qemu") {
          init_git()
          docker_run(ci_qemu, './tests/scripts/task_config_build_qemu.sh build', 'Create QEMU cmake config')
          try {
            make(ci_qemu, 'build', '-j2')
            cpp_unittest(ci_qemu)
            timeout(time: max_time, unit: 'MINUTES') {
              ci_setup(ci_qemu)
              docker_run(ci_qemu, ' ./tests/scripts/task_python_microtvm.sh', 'Run microTVM tests')
              docker_run(ci_qemu, ' ./tests/scripts/task_demo_microtvm.sh', 'Run microTVM demos')
            }
          } finally {
            junit 'build/pytest-results/*.xml'
          }
        }
      }
     } else {
      Utils.markStageSkippedForConditional('BUILD: QEMU')
    }
  },
  'BUILD: Hexagon': {
    if (!skip_ci && is_docs_only_build != 1) {
      node('CPU') {
        ws("workspace/exec_${env.EXECUTOR_NUMBER}/tvm/build-hexagon") {
          init_git()
          docker_run(ci_hexagon, './tests/scripts/task_config_build_hexagon.sh build', 'Create Hexagon cmake config')
          try {
            make(ci_hexagon, 'build', '-j2')
            cpp_unittest(ci_hexagon)
            docker_run(ci_hexagon, './tests/scripts/task_build_hexagon_api.sh', 'Build Hexagon API')
            docker_run(ci_hexagon, './tests/scripts/task_python_hexagon.sh', 'Run Hexagon tests')
            docker_run(ci_hexagon, './tests/scripts/task_python_hexagon_simulator.sh', 'Run Hexagon tests on simulator')
          } finally {
            junit 'build/pytest-results/*.xml'
          }
        }
      }
     } else {
      Utils.markStageSkippedForConditional('BUILD: Hexagon')
    }
  }
}

stage('Test') {
  environment {
    SKIP_SLOW_TESTS = "${skip_slow_tests}"
  }
  parallel 'unittest: GPU': {
    if (!skip_ci && is_docs_only_build != 1) {
      node('TensorCore') {
        ws("workspace/exec_${env.EXECUTOR_NUMBER}/tvm/ut-python-gpu") {
          try {
            init_git()
            unpack_lib('gpu2', tvm_multilib)
            cpp_unittest(ci_gpu)

            unpack_lib('gpu', tvm_multilib)
            timeout(time: max_time, unit: 'MINUTES') {
              ci_setup(ci_gpu)
              cpp_unittest(ci_gpu)
              docker_run(ci_gpu, './tests/scripts/task_java_unittest.sh', 'Run Java unit tests')
              docker_run(ci_gpu, './tests/scripts/task_python_unittest_gpuonly.sh', 'Run Python GPU unit tests')
              docker_run(ci_gpu, './tests/scripts/task_python_integration_gpuonly.sh', 'Run Python GPU integration tests')
            }
          } finally {
            junit 'build/pytest-results/*.xml'
          }
        }
      }
    } else {
      Utils.markStageSkippedForConditional('unittest: GPU')
    }
  },
  'integration: CPU': {
    if (!skip_ci && is_docs_only_build != 1) {
      node('CPU') {
        ws("workspace/exec_${env.EXECUTOR_NUMBER}/tvm/ut-python-cpu") {
          try {
            init_git()
            unpack_lib('cpu', tvm_multilib_tsim)
            timeout(time: max_time, unit: 'MINUTES') {
              ci_setup(ci_cpu)
              docker_run(ci_cpu, './tests/scripts/task_python_integration.sh', 'Run CPU integration tests')
            }
          } finally {
            junit 'build/pytest-results/*.xml'
          }
        }
      }
    } else {
      Utils.markStageSkippedForConditional('integration: CPU')
    }
  },
  'unittest: CPU': {
    if (!skip_ci && is_docs_only_build != 1) {
      node('CPU') {
        ws("workspace/exec_${env.EXECUTOR_NUMBER}/tvm/ut-python-cpu") {
          try {
            init_git()
            unpack_lib('cpu', tvm_multilib_tsim)
            timeout(time: max_time, unit: 'MINUTES') {
              ci_setup(ci_cpu)
              cpp_unittest(ci_cpu)
              python_unittest(ci_cpu)
              fsim_test(ci_cpu)
              docker_run(ci_cpu, './tests/scripts/task_python_vta_tsim.sh', 'Run VTA tests in TSIM')
            }
          } finally {
            junit 'build/pytest-results/*.xml'
          }
        }
      }
    } else {
      Utils.markStageSkippedForConditional('unittest: CPU')
    }
  },
  'python3: i386': {
    if (!skip_ci && is_docs_only_build != 1) {
      node('CPU') {
        ws("workspace/exec_${env.EXECUTOR_NUMBER}/tvm/ut-python-i386") {
          try {
            init_git()
            unpack_lib('i386', tvm_multilib)
            timeout(time: max_time, unit: 'MINUTES') {
              ci_setup(ci_i386)
              cpp_unittest(ci_i386)
              python_unittest(ci_i386)
              docker_run(ci_i386, ' ./tests/scripts/task_python_integration_i386only.sh', 'Run i386 integration tests')
              fsim_test(ci_i386)
            }
          } finally {
            junit 'build/pytest-results/*.xml'
          }
        }
      }
    } else {
      Utils.markStageSkippedForConditional('python3: i386')
    }
  },
  'python3: aarch64': {
    if (!skip_ci && is_docs_only_build != 1) {
      node('ARM') {
        ws("workspace/exec_${env.EXECUTOR_NUMBER}/tvm/ut-python-arm") {
          try {
            init_git()
            unpack_lib('arm', tvm_multilib)
            timeout(time: max_time, unit: 'MINUTES') {
              ci_setup(ci_arm)
              cpp_unittest(ci_arm)
              python_unittest(ci_arm)
              docker_run(ci_arm, './tests/scripts/task_python_arm_compute_library.sh', 'Run test_arm_compute_lib test')
              docker_run(ci_arm, './tests/scripts/task_python_topi.sh', 'Run TOPI tests')
              docker_run(ci_arm, './tests/scripts/task_python_integration.sh', 'Run CPU integration tests')
            }
          } finally {
            junit 'build/pytest-results/*.xml'
          }
        }
      }
    } else {
      Utils.markStageSkippedForConditional('python3: arm')
    }
  },
  'topi: GPU': {
    if (!skip_ci && is_docs_only_build != 1) {
      node('GPU') {
        ws("workspace/exec_${env.EXECUTOR_NUMBER}/tvm/topi-python-gpu") {
          try {
            init_git()
            unpack_lib('gpu', tvm_multilib)
            timeout(time: max_time, unit: 'MINUTES') {
              ci_setup(ci_gpu)
              docker_run(ci_gpu, './tests/scripts/task_python_topi.sh', 'Run TOPI tests')
            }
          } finally {
            junit 'build/pytest-results/*.xml'
          }
        }
      }
    } else {
      Utils.markStageSkippedForConditional('topi: GPU')
    }
  },
  'frontend: GPU 1': {
    if (!skip_ci && is_docs_only_build != 1) {
      node('GPU') {
        ws("workspace/exec_${env.EXECUTOR_NUMBER}/tvm/frontend-python-gpu") {
          try {
            init_git()
            unpack_lib('gpu', tvm_multilib)
            timeout(time: max_time, unit: 'MINUTES') {
              ci_setup(ci_gpu)
              docker_run(ci_gpu, './tests/scripts/task_python_frontend.sh 1', 'Run Python frontend tests (shard 1)')
            }
          } finally {
            junit 'build/pytest-results/*.xml'
          }
        }
      }
     } else {
      Utils.markStageSkippedForConditional('frontend: GPU 1')
    }
  },
  'frontend: GPU 2': {
    if (!skip_ci && is_docs_only_build != 1) {
      node('GPU') {
        ws("workspace/exec_${env.EXECUTOR_NUMBER}/tvm/frontend-python-gpu") {
          try {
            init_git()
            unpack_lib('gpu', tvm_multilib)
            timeout(time: max_time, unit: 'MINUTES') {
              ci_setup(ci_gpu)
              docker_run(ci_gpu, './tests/scripts/task_python_frontend.sh 2', 'Run Python frontend tests (shard 2)')
            }
          } finally {
            junit 'build/pytest-results/*.xml'
          }
        }
      }
     } else {
      Utils.markStageSkippedForConditional('frontend: GPU 2')
    }
  },
  'frontend: CPU': {
    if (!skip_ci && is_docs_only_build != 1) {
      node('CPU') {
        ws("workspace/exec_${env.EXECUTOR_NUMBER}/tvm/frontend-python-cpu") {
          try {
            init_git()
            unpack_lib('cpu', tvm_multilib)
            timeout(time: max_time, unit: 'MINUTES') {
              ci_setup(ci_cpu)
              docker_run(ci_cpu, './tests/scripts/task_python_frontend_cpu.sh', 'Run Python frontend tests')
            }
          } finally {
            junit 'build/pytest-results/*.xml'
          }
        }
      }
    } else {
      Utils.markStageSkippedForConditional('frontend: CPU')
    }
  },
  'docs: GPU': {
    if (!skip_ci) {
      node('TensorCore') {
        ws("workspace/exec_${env.EXECUTOR_NUMBER}/tvm/docs-python-gpu") {
          init_git()
          unpack_lib('gpu', tvm_multilib)
          timeout(time: max_time, unit: 'MINUTES') {
            ci_setup(ci_gpu)
            docker_run(ci_gpu, './tests/scripts/task_python_docs.sh', 'Build docs')
          }
          pack_lib('docs', 'docs.tgz')
          archiveArtifacts(artifacts: 'docs.tgz', fingerprint: true)
        }
      }
    }
  }
}

/*
stage('Build packages') {
  parallel 'conda CPU': {
    node('CPU') {
      sh "${docker_run} tlcpack/conda-cpu ./conda/build_cpu.sh
    }
  },
  'conda cuda': {
    node('CPU') {
      sh "${docker_run} tlcpack/conda-cuda90 ./conda/build_cuda.sh
      sh "${docker_run} tlcpack/conda-cuda100 ./conda/build_cuda.sh
    }
  }
// Here we could upload the packages to anaconda for releases
// and/or the main branch
}
*/

def deploy_docs() {
  // Note: This code must stay in the Jenkinsfile to ensure that it runs
  // from a trusted context only
  sh(
    script: '''
      set -eux
      rm -rf tvm-site
      git clone -b $DOCS_DEPLOY_BRANCH --depth=1 https://github.com/apache/tvm-site
      cd tvm-site
      git status
      git checkout -B $DOCS_DEPLOY_BRANCH

      rm -rf docs
      mkdir -p docs
      tar xf ../docs.tgz -C docs
      COMMIT=$(cat docs/commit_hash)
      git add .
      git config user.name tvm-bot
      git config user.email 95660001+tvm-bot@users.noreply.github.com
      git commit -m"deploying docs (apache/tvm@$COMMIT)"
      git status
    ''',
    label: 'Unpack docs and update tvm-site'
  )

  withCredentials([string(
    credentialsId: 'docs-push-token',
    variable: 'GITHUB_TOKEN',
    )]) {
    sh(
      script: '''
        cd tvm-site
        git remote add deploy https://$GITHUB_TOKEN:x-oauth-basic@github.com/apache/tvm-site.git
        git push deploy $DOCS_DEPLOY_BRANCH
      ''',
      label: 'Upload docs to apache/tvm-site'
    )
  }
}

stage('Deploy') {
  if (env.BRANCH_NAME == 'main' && env.DOCS_DEPLOY_ENABLED == 'yes') {
    node('CPU') {
      ws("workspace/exec_${env.EXECUTOR_NUMBER}/tvm/deploy-docs") {
        unpack_lib('docs', 'docs.tgz')
        deploy_docs()
      }
    }
  }
}
