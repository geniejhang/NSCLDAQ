#!/usr/bin/env python3

from subprocess import call, Popen, PIPE
import argparse
import os

versionOpts = [8, 10, 11]

def set_up_parser():
  parser = argparse.ArgumentParser(description='Convert a file between two data formats')
  parser.add_argument('--inputformat', type=int, dest='inputformat', choices=versionOpts, help='Version of input data')
  parser.add_argument('--outputformat', type=int, dest='outputformat', choices=versionOpts, help='Version of output data')
  parser.add_argument('--source', dest='source', help='Data source url (stdin="-", file=file:///path/to/data, ring=tcp://host/ringname)')
  parser.add_argument('--sink', dest='sink', help='Data sink url (stdout="-", file=file:///path/to/data, ring=tcp://host/ringname)')
  return parser

def compute_conversion_stages(input, output):
  inIndex = versionOpts.index(input)
  outIndex = versionOpts.index(output)
  print(inIndex, ' ' , outIndex)
  
  stages = []
  if outIndex > inIndex :
    for index in range(inIndex, outIndex, 1):
      stages.append({'in':versionOpts[index], 'out':versionOpts[index+1]})
  else:
    for index in range(inIndex, outIndex, -1):
      stages.append({'in':versionOpts[index], 'out':versionOpts[index-1]})

  print(stages)
  return stages

def executable():
  bin_dir = os.path.dirname(os.path.realpath(__file__))
  return os.path.join(bin_dir, 'format_converter')
  
def set_up_simple_command(stage, source, sink):
  print("set up simple command")
  cmd = [executable(), 
      '--input-version', str(stage['in']), 
      '--output-version', str(stage['out']), 
      '--source', source, 
      '--sink', sink]
  call(cmd, shell=False)

def set_up_compound_command(stages, source, sink):
  print("set up compound command")
  cmd = [executable(), 
      '--input-version', str(stages[0]['in']), 
      '--output-version', str(stages[0]['out']), 
      '--source', source, 
      '--sink', '-']
  p = []
  p.append(Popen(cmd, shell=False, stdout=PIPE))
  for stage in stages[1:len(stages)-2]:
    cmd = [executable(), 
        '--input-version', str(stage['in']), 
        '--output-version', str(stage['out']), 
        '--source', '-',
        '--sink', '-']
    p.append(Popen(cmd, stdin=p[-1].stdout, stdout=PIPE))
  
  cmd = [executable(), 
      '--input-version', str(stages[-1]['in']), 
      '--output-version', str(stages[-1]['out']), 
      '--source', '-', 
      '--sink', sink]
  p.append(Popen(cmd, stdin=p[-1].stdout, stdout=PIPE))
  dummy = input()
  p[-1].wait()




# Should build a single stage like:
# format_converter --input-version 8 --output-version 10 --source=... --sink=...
# or a double stage like:
# format_converter --input-version 8 --output-version 10 --source=... | format_converter --input-version 10 --output-version 11 --sink=...
def build_pipeline(args):
  stages = compute_conversion_stages(args.inputformat, args.outputformat)
  
  if len(stages) == 1:
    set_up_simple_command(stages[0], args.source, args.sink)
  elif len(stages) > 1:
    set_up_compound_command(stages, args.source, args.sink)
  else:
    raise RuntimeError()

def main() :
  parser = set_up_parser()
  args = parser.parse_args()
  pipeline = build_pipeline(args)

if __name__ == '__main__' :
    main()
