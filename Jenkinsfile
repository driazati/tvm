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

// https://www.jenkins.io/doc/book/pipeline/syntax/#declarative-pipeline
// wow

void initGit() {
  checkout scm
  // Add more info about job node
  sh(
    script: './tests/scripts/task_show_node_info.sh',
    label: 'Show executor node info',
  )
  retry(5) {
    timeout(time: 2, unit: 'MINUTES') {
      sh(
        script: 'git submodule update --init -f',
        label: 'Update git submodules'
      )
    }
  }
}

void docker(test) {
  // sh(
  //   script: "./docker/bash.sh ${image} ${command}",
  //   label: label
  // )
  sh(
    script: "echo ${test}",
    label: 'say wow'
  )
}

def shouldSkipCI(prNumber) {
  withCredentials([string(
    credentialsId: 'tvm-bot-jenkins-reader',
    variable: 'TOKEN',
  )]) {
    // Exit code of 1 means run full CI (or the script had an error, so run
    // full CI just in case). Exit code of 0 means skip CI.
    gitSkipCICode = sh(
      returnStatus: true,
      script: "./tests/scripts/git_skip_ci.py --pr '${prNumber}'",
      label: 'Check if CI should be skipped',
    )
  }
  return gitSkipCICode == 0
}

def skipCI = false
def isDocsOnlyBuild = false

pipeline {
  agent none
  parameters {
    string(
      name: 'ci_lint',
      defaultValue: 'tlcpack/ci-lint:v0.68',
      description: 'ci-lint Docker image')
  }
  options {
    preserveStashes()
  }
  stages {
    stage('Lint') {
      agent {
        node {
          label 'CPU'
          customWorkspace "workspace/exec_${env.EXECUTOR_NUMBER}/lint"
        }
      }
      steps {
        echo 'Hello World'
        sh(script: """
          echo "Docker images being used in this build:"
          echo " ci_lint = ${params.ci_lint}"
        """, label: 'Docker image names')
        initGit()
        // docker(
        //   script: './tests/scripts/task_lint.sh',
        //   image: params.ci_lint,
        //   label: 'Run lint'
        // )
        docker(test: 'wow')
        script {
          skipCI = shouldSkipCI(env.CHANGE_ID)
          isDocsOnlyBuild = sh(
            returnStatus: true,
            script: './tests/scripts/git_change_docs.sh',
            label: 'Check for docs only changes',
          )
        }
      // docker(
      //   script: './tests/scripts/task_lint.sh',
      //   image: params.ci_lint,
      //   label: 'Run lint'
      // )
      }
    }
    stage('Build') {
      when { expression { return !skipCI } }
      parallel {
        stage('CPU') {
          agent {
            node {
              label 'CPU'
              customWorkspace "workspace/exec_${env.EXECUTOR_NUMBER}/cpu"
            }
          }
          steps {
            sh 'echo wowowo'
          }
        }
        stage('GPU') {
          agent {
            node {
              label 'GPU'
              customWorkspace "workspace/exec_${env.EXECUTOR_NUMBER}/gpu"
            }
          }
          steps {
            sh 'echo wowowo'
          }
        }
      }
    }
  }
}
