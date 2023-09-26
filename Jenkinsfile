@Library('sqdr-jenkins-lib') _

git_repository = "sqdr-g3log"
github_credentials_id = "github_personnal_access_token"
git_credentials_id = "ci-squadrone"
build_dirs = ['.']
Map nodes = [
    "amd64/ubuntu/focal": "ec2-fleet-amd64",
    "amd64/ubuntu/jammy": "ec2-fleet-amd64",
    //"amd64/debian/buster": "ec2-fleet-amd64",
    "amd64/debian/bullseye": "ec2-fleet-amd64",
    "arm64/ubuntu/focal": "ec2-fleet-arm64",
    "arm64/ubuntu/jammy": "ec2-fleet-arm64",
    //"arm64/debian/buster": "ec2-fleet-arm64",
    "arm64/debian/bullseye": "ec2-fleet-arm64",
    "armhf/raspbian/buster": "ec2-fleet-arm64",
    "armhf/rapsbian/bullseye": "ec2-fleet-arm64"
]

Map tasks = [:]
nodes.each { build_arch, label ->
    tasks[build_arch] = {
        node(label) {
            try {
                stage('Checkout SCM') {
                    if (env.CHANGE_BRANCH) {
                        env.setProperty('BRANCH_NAME', env.CHANGE_BRANCH)
                    }
                    cleanWs()
                    checkout([$class: 'GitSCM', branches: [[name: "${env.BRANCH_NAME}"]], doGenerateSubmoduleConfigurations: false, extensions: [[$class: 'SubmoduleOption', disableSubmodules: false, parentCredentials: true, recursiveSubmodules: true, reference: '', trackingSubmodules: false], [$class: 'CloneOption', noTags: false, reference: '', shallow: true]], submoduleCfg: [], userRemoteConfigs: [[credentialsId: git_credentials_id, url: "git@github.com:SquadroneSystem/${git_repository}.git"]]])
                }

                stage('Notify GitHub build start') {
                    notifyGithub.buildStatus(arch: build_arch, status: "pending")
                }

                sqdrBuild.debianPackages(build_arch: build_arch, build_dirs: build_dirs)

                if (env.BRANCH_NAME == 'sqdr-main') {
                    sqdrDeploy.debianPackages(build_arch: build_arch)
                }

                stage("Archive debian packages") {
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
                        notifyGithub.buildStatus(arch: build_arch, status: "success")
                    } else {
                        echo "Failed to build deb packages"
                        notifyGithub.buildStatus(arch: build_arch, status: "failure")
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
