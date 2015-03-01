# vim: filetype=python
import re

APPNAME = 'watson'
VERSION = '0.1.0'
top = '.'
out = 'build'

def options(opt):
    opt.load('compiler_cxx')
    opt.load('waf_unit_test')

def configure(conf):
    conf.load('compiler_cxx')
    conf.load('waf_unit_test')

    conf.check(
        header_name='snappy.h'
        ,lib=['snappy']
        ,libpath=[
            '/usr/local/lib'
            ,'/usr/lib'
        ]
        ,includes=[
            '/usr/local/include'
            ,'/usr/include'
        ]
        ,mandatory=True
    )

    conf.write_config_header('config.h')


def build(bld):
    bld.load('compiler_cxx')
    bld.load('waf_unit_test')

    from waflib.Tools import waf_unit_test
    bld.add_post_fun(waf_unit_test.summary)

    #Build the watson spec library.
    bld.shlib(
        features=[
            'cxx'
        ]
        ,source=[
            'src/watson.cpp'
        ]
        ,target='watson'
        ,cxxflags=[
            '-O0'
            ,'-Wall'
            ,'-g'
            ,'-std=c++11'
            ,'-stdlib=libc++'
        ]
        ,includes=[
            './src'
        ]
        ,linkflags=[
            '-g'
            ,'-std=c++11'
            ,'-stdlib=libc++'
        ]
        ,use = [
            'SNAPPY.H'
        ]

    )

    # preform the unit tests
    test_nodes = bld.path.ant_glob('test/**/*.cpp')
    for node in test_nodes:
        bld(
            features = [
                'cxx'
                ,'cxxprogram'
                ,'test'
            ]
            ,includes = [
                './test'
                ,'./src'
            ]
            ,source = [node]
            ,target = node.change_ext('')
            ,use = [
                'watson'
                ,'SNAPPY.h'
            ]
            ,cxxflags = [
                '-O0'
                ,'-Wall'
                ,'-g'
                ,'-std=c++11'
                ,'-stdlib=libc++'
            ]
            ,linkflags = [
                '-g'
                ,'-std=c++11'
                ,'-stdlib=libc++'
            ]
        )

