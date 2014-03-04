# Get to the Travis build directory, configure git and clone the repo
cd $HOME
git config --global user.email "travis@travis-ci.org"
git config --global user.name "travis-ci"
git clone --quiet --branch=gh-pages https://${GH_TOKEN}@github.com/ryanhope/gazetools gh-pages > /dev/null

# Commit and Push the Changes
cd gh-pages
git rm -rf *
cp -Rf $HOME/inst/web/* .
git add -A
git commit -m "Lastest staticdoc on successful travis build $TRAVIS_BUILD_NUMBER auto-pushed to gh-pages"
git push -fq origin gh-pages > /dev/null