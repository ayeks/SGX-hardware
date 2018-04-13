require 'rubygems'
require 'bundler'
Bundler.require(:default)

desc "Run Mardown validation for the repository"
task :validate_markdown do
  MarkdownProofer::RakeTask.run
end

task default: :validate_markdown
