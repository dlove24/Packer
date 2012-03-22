# Copyright (c) 2011 David Love
#
# Permission to use, copy, modify, and/or distribute this software for 
# any purpose with or without fee is hereby granted, provided that the 
# above copyright notice and this permission notice appear in all copies.
#
# THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES 
# WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF 
# MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR 
# ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES 
# WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN 
# ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF 
# OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
#
# encoding: utf-8

###
### Set-up. Actions to run before all tasks
###

require 'rubygems'
require 'rake'
require 'bundler'
require 'jeweler'

## Use Bundler to make sure the gem environment is up-to-date
begin
  Bundler.setup(:default, :development)
rescue Bundler::BundlerError => e
  $stderr.puts e.message
  $stderr.puts "Run `bundle install` to install missing gems"
  exit e.status_code
end

## Versions are managed by Jeweler rake tasks
Jeweler::Tasks.new

###
### Testing. Run the unit/system/integration tests
###

require 'rake/testtask'

desc "Run all our tests"
task :test do
  Rake::TestTask.new do |t|
    t.libs << "test"
    t.verbose = false
    
    # List of files. We need to specify these explicitly so they
    # are loaded in the correct order: init, data, then the tests
    # themselves
    t.test_files = ["test/init_test.rb",
                    
                    "test/data/data_test.rb"]
                    
                    #test/data_structures/uuid_test.rb"]
  end
end

###
### Re-Style. Make C code style consistent
###

## Use astyle to ensure consistency in the C code 
desc "Ensure the C libraries are written in a consistent style"
task :style do
  sh "astyle  --style=banner --indent=spaces=2 --indent-preprocessor --indent-col1-comments --break-blocks=all --pad-oper --pad-paren-out --unpad-paren --add-brackets --convert-tabs --align-pointer=type  --recursive src/*.c src/*.cpp src/*.h src/*.c"
# --align-reference=name
end

###
### Documentation. Generate/update the project documentation
###

## Use Doxygen to generate the documentations
desc "Generate Doxygen HTML documentation from the Lua/C sources"
task :document do
  sh "doxygen Doxyfile"
end
## Set documentation generation as the default action
task :default => :document

###
### Release. Additional actions to take when releasing the gem
###

## Additional actions for the release task
desc "Update the project HISTORY file from the commit log"
task :history do
  # Update the HISTORY file
  sh "vclog history -f gnu  > HISTORY"
  sh "git commit -a -m \"doc: Update HISTORY file\""
end

###
### Clean. Clean-up the project directory
###

require 'rake/clean'

# Remove any Doxygen generated files
CLOBBER.include('doc') 

###
### Github. Manage the local and remote source code repository
###

require 'readline'

def git_branch
  `git branch | grep "*"`.strip[2..-1]
end

def compare_git_ver
  m = /git version (\d+).(\d+).(\d+)/.match(`git version`.strip)
  return true  if m[1].to_i > 1
  return false if m[1].to_i < 1
  return true  if m[2].to_i > 5
  return false if m[2].to_i < 5
  return true  if m[3].to_i >= 3
  return false
end

def check_git_ver
  raise "Invalid git version, use at least 1.5.3" unless compare_git_ver
end

namespace :git do

  desc "Show the current status of the checkout"
  task :status do
    system "git status"
  end

  desc "Create a new topic branch"
  task :topic do
    total = `git branch`.scan("quick").size
    if total == 0
      default = "quick"
    else
      default = "quick#{total + 1}"
    end
    name = Readline.readline "Topic name (default #{default}): "
    if name.strip.empty?
      name = default
    end
    sh "git checkout -b #{name}"
  end

  desc "Push all changes to the repository"
  task :push => :update do
    branch = git_branch()
    if branch != "master"
      `git diff-files --quiet`
      if $?.exitstatus == 1
        puts "You have outstanding changes. Please commit them first."
        exit 1
      end

      puts "* Merging topic '#{branch}' back into master..."
      `git checkout master`
      sh "git merge #{branch}"
      switch = true
    else
      switch = false
    end

    puts "* Pushing changes..."
    sh "git push"

    if switch
      puts "* Switching back to #{branch}..."
      `git checkout #{branch}`
    end
  end

  desc "Pull new commits from the repository"
  task :update do
    check_git_ver
    `git diff-files --quiet`
    if $?.exitstatus == 1
      stash = true
      clear = `git stash list`.scan("\n").size == 0
      puts "* Saving changes..."
      `git stash save`
    else
      stash = false
    end

    branch = git_branch()
    if branch != "master"
      switch = true
      `git checkout master`
      puts "* Switching back to master..."
    else
      switch = false
    end

    puts "* Pulling in new commits..."
    sh "git fetch"
    sh "git rebase origin"

    if switch
      puts "* Porting changes into #{branch}..."
      `git checkout #{branch}`
      sh "git rebase master"
    end

    if stash
      puts "* Applying changes..."
      sh "git stash apply"
      `git stash clear` if clear
    end
  end

  task :pull => :update
end
