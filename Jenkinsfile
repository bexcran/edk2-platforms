pipeline {
    agent none
    options { skipDefaultCheckout() }

    stages {
        stage('Stash binaries') {
            agent { label 'controller' }
            steps {
                echo 'Stashing binaries for building final firmware image'
            }
        }
        stage('Setup Environment') {
            agent { label 'x86_64' }
            steps {
                script {
                    def nodeName = "${NODE_NAME}"
                    echo "Running on node: ${nodeName}"
                }
                script {
                  if (fileExists('trusted-firmware-a/readme.rst')) {
                    dir('trusted-firmware-a') {
                      sh 'git pull'
                      sh 'git checkout -- .'
                      sh 'git clean -fdx'
                    }
                  } else {
                    sh 'rm -rf trusted-firmware-a && git clone --depth 1 https://git.trustedfirmware.org/TF-A/trusted-firmware-a.git'
                  }
                }
                sh ''' 
                  make -C trusted-firmware-a fiptool
                  make -C trusted-firmware-a certtool
                '''
                dir("trusted-firmware-a/tools/fiptool") {
                  stash name: "fiptool", includes: "fiptool"
                }
                dir("trusted-firmware-a/tools/cert_create") {
                  stash name: "certtool", includes: "cert_create"
                }

                echo "Downloading and building ACPICA (ACPI Component Architecture) 20250404"
                sh 'wget -N -nd --unlink https://github.com/acpica/acpica/releases/download/R2025_04_04/acpica-unix-20250404.tar.gz'
                sh 'tar xf acpica-unix-20250404.tar.gz'
                dir('acpica-unix-20250404') {
                  sh 'make -j $(nproc)'
                }
                dir("acpica-unix-20250404/generate/unix/bin") {
                  stash name: "acpica", includes: "*"
                }
            }
        }
        stage('Build') {
            agent { label 'x86_64' }
            steps {
                script {
                    def nodeName = "${NODE_NAME}"
                    echo "Running on node: ${nodeName}"
                }

                echo 'Building...'
                checkout scmGit(branches: [[name: env.branch]], extensions: [[$class: 'RelativeTargetDirectory', relativeTargetDir: 'edk2-platforms']], userRemoteConfigs: [[url: env.repo]])
                script {
                  if (fileExists('edk2-ampere-tools/nvparam.py')) {
                    dir('edk2-ampere-tools') {
                      sh 'git pull'
                      sh 'git checkout -- .'
                      sh 'git clean -fdx'
                    }
                  } else {
                    sh 'rm -rf edk2-ampere-tools && git clone --depth 1 https://github.com/AmpereComputing/edk2-ampere-tools.git'
                  }
                }
                script {
                  if (fileExists('edk2/BaseTools/Edk2ToolsBuild.py')) {
                    dir('edk2') {
                      sh 'git pull'
                      sh 'git checkout -- .'
                      sh 'git clean -fdx' 
                    }
                  } else {
                    sh 'rm -rf edk2 && git clone https://github.com/tianocore/edk2.git'
                  }
                }
                script {
                  if (fileExists('edk2-non-osi/Readme.md')) {
                    dir('edk2-non-osi') {
                      sh 'git pull'
                      sh 'git checkout -- .'
                      sh 'git clean -fdx'
                    }
                  } else {
                   sh 'rm -rf edk2-non-osi && git clone https://github.com/tianocore/edk2-non-osi'
                  }
                }

                dir('edk2') {
                  sh 'git checkout ' + env.edk2revision
                  sh 'git submodule update --init --force'
                }
                unstash "fiptool"
                unstash "certtool"
                unstash "acpica"
                withEnv(['PATH+TOOLS=' + pwd()]) {
                  sh 'echo PATH is $PATH'
                  sh '''
                    export PYTHON_COMMAND=python3
                    export GCC_AARCH64_PREFIX=aarch64-linux-gnu-
                    ./edk2-platforms/Platform/Ampere/buildfw.sh -m ASRockRack -p Altra1L2T -t GCC -b RELEASE
                  '''
                }
            }
        }
        stage('Test') {
            agent { label 'x86_64' }
            steps {
                echo 'Testing..'
            }
        }
        stage('Deploy') {
            agent { label 'x86_64' }
            steps {
                echo 'Deploying....'
            }
        }
    }
}
