# Initialize git repository (if not already done)
git init

# Add the remote repository
git remote add origin git@github.com:Ekpenyong-Esu/BareMetalMCU2.git

# Add all files to staging
git add .

# Commit your changes
git commit -m "Initial commit"

# Rename local master branch to mainch (instead of main)
git branch -m master main origin master

# Push to main branch
git push --set-upstream origin main
