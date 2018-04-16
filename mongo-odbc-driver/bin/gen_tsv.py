#!/usr/bin/env python
import copy
import sys
import yaml

def main():
    ret = ''
    if len(sys.argv) < 2:
        print "Usage gen_tsv.py <yaml file>"
        return False

    y = yaml.load(open(sys.argv[1]))

    if not  isinstance(y, list):
        print "The yml file top level must be an array"
        return False

    required_keys = set(['test_name', 'props'])
    valid_keys = copy.copy(required_keys)
    valid_keys.add('expected_err')
    for i, elem in enumerate(y):
        keys = elem.keys()
        for key in keys:
            if key not in valid_keys:
                print "%s is not a valid key for a test in test #: %d, valid keys are {%s}"%(key, i, ",".join(keys))
                return False
        for required_key in required_keys:
            if required_key not in keys:
                print "each test must have a '%s' key, missing in test #: %d"%(required_key, i)
                return False
        ret += elem['test_name']
        ret += '\t' + '\t'.join(elem['props'])
        if 'expected_err' in elem:
            ret += '\t' + elem['expected_err']
        ret += '\n'
    return ret

if __name__ == '__main__':
    out = main()
    if out:
        sys.stdout.write(out)
        sys.stdout.flush()
