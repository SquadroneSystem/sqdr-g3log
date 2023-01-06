def notifyGithub(build_arch, status) {
    try {
        sh "ci/notify_github $ACCESS_TOKEN $BUILD_URL $build_arch $status"
    } catch (err) {
        unstable(message: "notifyGithub($build_arch, $status) failed")
    }
}

Map nodes = [
    "amd64/ubuntu/focal": "ec2-fleet-amd64",
    "arm64/ubuntu/focal": "ec2-fleet-arm64",
    "armhf/raspbian/buster": "ec2-fleet-arm64"
]

Map tasks = [:]

// for each example: https://gist.github.com/oifland/ab56226d5f0375103141b5fbd7807398
nodes.each { build_arch, label ->
    tasks[build_arch] = {
        node(label) {
            try {
                stage('Checkout SCM') {
                    if (env.CHANGE_BRANCH) {
                        env.setProperty('BRANCH_NAME', env.CHANGE_BRANCH)
                    }
                    cleanWs()
                    checkout([$class: 'GitSCM', branches: [[name: "${env.BRANCH_NAME}"]], doGenerateSubmoduleConfigurations: false, extensions: [[$class: 'SubmoduleOption', disableSubmodules: false, parentCredentials: true, recursiveSubmodules: true, reference: '', trackingSubmodules: false], [$class: 'CloneOption', noTags: false, reference: '', shallow: true]], submoduleCfg: [], userRemoteConfigs: [[credentialsId: 'ci-squadrone', url: 'git@github.com:SquadroneSystem/sqdr-g3log.git']]])
                }

                stage('Environment Initialization') {
                    def git_commit = sh (script: 'git rev-parse HEAD', returnStdout: true).trim()
                    env.setProperty('GIT_COMMIT', git_commit)
                    env.setProperty('ACCESS_TOKEN', "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")
                    stage('Notify GitHub build start') {
                        notifyGithub(build_arch, "pending")
                    }

                    env.setProperty('BUILD_ARCH', build_arch)
                }

                stage('Create deb packages') {
                    if (env.BRANCH_NAME == 'sqdr-main') {
                        echo "Building main branch"
                        sh "PATH=/opt/sqdr/ssap/bin:$PATH \
                            jenkins-multi-venv.sh \
                                --build $BUILD_ARCH"
                    } else {
                        echo "Building test ${env.BRANCH_NAME}, packages will not be uploaded to apt repository"
                        sh "SSAP_JENKINS_DISABLE_DPUT=1 \
                            PATH=/opt/sqdr/ssap/bin:$PATH \
                            jenkins-multi-venv.sh \
                                --build $BUILD_ARCH"
                    }
                }

                stage('Archive build') {
                    echo "Archive artifacts:"
                    archiveArtifacts artifacts: "**/*jenkins${BUILD_NUMBER}*.deb", followSymlinks: true
                }
            } catch (e) {
                echo "Error: ${e}"
                echo "Failed to build deb packages"
                throw e
            } finally {
                stage('Notify result') {
                    def currentResult = currentBuild.result ?: 'SUCCESS'
                    if (currentResult == 'UNSTABLE') {
                        echo "Build is unstable"
                    }

                    if (currentResult == 'SUCCESS') {
                        echo "Finished building deb packages"
                        notifyGithub(build_arch, "success")
                    } else {
                        echo "Failed to build deb packages"
                        notifyGithub(build_arch, "failure")
                    }

                    def previousResult = currentBuild.getPreviousBuild()?.result
                    if (previousResult != null && previousResult != currentResult) {
                        echo "Build result changed from ${previousResult} to ${currentResult}"
                    }
                }

                stage('Clean workspace') {
                    cleanWs()
                }
            }
        }
    }
}

parallel(tasks)
