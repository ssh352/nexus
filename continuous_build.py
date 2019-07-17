import argparse
import distutils
from distutils import dir_util
import errno
import git
import os
import shutil
import subprocess
import sys
import tarfile
import time
import zipfile

def call(command, cwd=None):
  return subprocess.Popen(command.split(), stdout=subprocess.PIPE, cwd=cwd,
    stderr=subprocess.PIPE).communicate()

def makedirs(path):
  try:
    os.makedirs(path)
  except OSError as e:
    if e.errno != errno.EEXIST:
      raise

def make_tarfile(source, destination):
  with tarfile.open(destination, "w:gz") as tar:
    for file in os.listdir(source):
      tar.add(os.path.join(source, file), arcname=file)

def make_zipfile(source, destination):
  archive = zipfile.ZipFile(destination, 'w', zipfile.ZIP_DEFLATED)
  source = os.path.abspath(source)
  for root, dirs, files in os.walk(source):
    for file in files:
      absolute_path = os.path.abspath(os.path.join(root, file))
      archive.write(os.path.join(root, file),
        arcname=absolute_path[len(source) + 1:])
  archive.close()

def copy_build(applications, version, name, source, path):
  try:
    destination_path = os.path.join(path, str(version))
    for application in applications:
      application_path = os.path.join(destination_path, application)
      makedirs(application_path)
      source_directory = os.path.join(source, 'Applications', application,
        'Application')
      for file in os.listdir(source_directory):
        file_path = os.path.join(source_directory, file)
        if os.path.isdir(file_path):
          shutil.copytree(file_path, os.path.join(application_path, file))
          continue
        if not os.path.isfile(file_path):
          continue
        extension = os.path.splitext(file_path)[1]
        if extension in ['.py', '.yml']:
          shutil.copy2(file_path, os.path.join(application_path, file))
        if sys.platform == 'win32':
          if extension in ['.bat', '.exe']:
            shutil.copy2(file_path, os.path.join(application_path, file))
        else:
          if extension in ['', '.sh']:
            shutil.copy2(file_path, os.path.join(application_path, file))
    library_destination_path = os.path.join(destination_path, 'Libraries')
    makedirs(library_destination_path)
    library_source_path = os.path.join(source, name, 'Libraries', 'Release')
    for file in os.listdir(library_source_path):
      shutil.copy2(os.path.join(library_source_path, file),
        os.path.join(library_destination_path, file))
  except OSError:
    return

def build_repo(repo, path, branch):
  commits = repo.git.rev_list('--first-parent', 'HEAD').split('\n')
  commits.reverse()
  builds = [int(d) for d in os.listdir(path) if os.path.isdir(
    os.path.join(path, d))]
  builds.sort(reverse=True)
  if len(builds) == 0:
    commits = commits[-1]
  else:
    for i in range(len(commits) - 1, -1, -1):
      version = int(repo.git.rev_list('--count', '--first-parent', commits[i]))
      if version in builds:
        commits = commits[i + 1:]
        break
  if sys.platform == 'win32':
    extension = 'bat'
  else:
    extension = 'sh'
  for commit in commits:
    version = int(repo.git.rev_list('--count', '--first-parent', commit))
    repo.git.checkout(commit)
    result = []
    result.append(call(
      os.path.join(repo.working_dir, 'configure.%s' % extension) + ' -DD=%s' %
      os.path.join(os.getcwd(), 'Dependencies')), repo.working_dir)
    result.append(call(os.path.join(repo.working_dir, 'build.%s' % extension),
      repo.working_dir))
    terminal_output = b''
    for output in result:
      terminal_output += output[0] + b'\n\n\n\n'
    for output in result:
      terminal_output += output[1] + b'\n\n\n\n'
    destination_path = os.path.join(path, str(version))
    makedirs(destination_path)
    nexus_applications = ['AdministrationServer', 'AsxItchMarketDataFeedClient',
      'ChartingServer', 'ChiaMarketDataFeedClient', 'ComplianceServer',
      'CseMarketDataFeedClient', 'CtaMarketDataFeedClient', 'DefinitionsServer',
      'MarketDataRelayServer', 'MarketDataServer', 'RiskServer',
      'SimulationMarketDataFeedClient', 'SimulationOrderExecutionServer',
      'TmxIpMarketDataFeedClient', 'TmxTl1MarketDataFeedClient',
      'UtpMarketDataFeedClient', 'WebPortal']
    if sys.platform == 'win32':
      nexus_applications.append('Spire')
    copy_build(nexus_applications, version, 'Nexus', repo.working_dir, path)
    beam_applications = ['AdminClient', 'RegistryServer', 'ServiceLocator',
      'UidServer']
    beam_path = os.path.join(os.getcwd(), 'Dependencies', 'Beam')
    copy_build(beam_applications, version, 'Beam', beam_path, path)
    shutil.copy2(os.path.join(repo.working_dir, 'Applications', 'setup.py'),
      os.path.join(destination_path, 'setup.py'))
    if sys.platform == 'win32':
      archive_path = os.path.join(path, 'nexus-%s.zip' % str(version))
      make_zipfile(destination_path, archive_path)
    else:
      for file in ['check.sh', 'copy_all.sh', 'start.sh', 'stop.sh']:
        shutil.copy2(os.path.join(repo.working_dir, 'Applications', file),
          os.path.join(destination_path, file))
      archive_path = os.path.join(path, 'nexus-%s.tar.gz' % str(version))
      make_tarfile(destination_path, archive_path)
    shutil.rmtree(destination_path)
    makedirs(destination_path)
    with open(os.path.join(destination_path, 'build.txt'), 'wb') as log_file:
      log_file.write(terminal_output)
    shutil.move(archive_path, destination_path)

def main():
  parser = argparse.ArgumentParser(
    description='v1.0 Copyright (C) 2019 Eidolon Systems Ltd.')
  parser.add_argument('-r', '--repo', type=str, help='Remote repository.',
    default='https://github.com/eidolonsystems/nexus.git')
  parser.add_argument('-p', '--path', type=str, help='Destination path.',
    required=True)
  parser.add_argument('-b', '--branch', type=str, help='Branch to build.',
    default='master')
  parser.add_argument('-t', '--period', type=int, help='Time period.',
    default=600)
  args = parser.parse_args()
  repo_path = os.path.join(os.getcwd(), 'Nexus')
  shutil.rmtree(repo_path, True)
  repo = git.Repo.clone_from(args.repo, repo_path)
  makedirs(args.path)
  while True:
    try:
      repo.git.checkout(args.branch)
      repo.git.pull()
    except:
      print('Failed to pull: ', sys.exc_info()[0])
    build_repo(repo, args.path, args.branch)
    time.sleep(args.period)

if __name__ == '__main__':
  main()
