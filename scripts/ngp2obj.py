#!/usr/bin/python

import sys
import getopt
import os.path
import _ngp

ScriptVersion = '0.9.2'

def ShowUsage():
    print('Usage: ngp2obj.py [options] source dest')
    print()
    print('Convert .ngp-file \'source\' to .obj-file \'dest\'')
    print()
    print('Options:')
    print(' -h               - display this help and exit')
    print(' -v               - display version information and exit')
    print(' -t TEXPATH       - prepend TEXPATH to all texture names')
    print()

def OnInvCmdLine():
    sys.stderr.write('error: invalid command line - use \'-h\' option to get help\n')
    sys.exit(1)

try:
    Options,Args = getopt.getopt(sys.argv[1:],'hvt:')
except:
    OnInvCmdLine()

SourceFileName    = None
TargetOBJFileName = None
TargetMTLFileName = None
TexturePathPrefix = ''

for Option,Value in Options:
    if   Option == '-v':
        print(ScriptVersion)
        sys.exit(0)
    elif Option == '-h':
        ShowUsage();
        sys.exit(0)
    elif Option == '-t':
        TexturePathPrefix = Value

if len(Args) != 2:
    OnInvCmdLine()

SourceFileName    = Args[0]
TargetOBJFileName = Args[1]
BaseName,ExtName  = os.path.splitext(TargetOBJFileName)
TargetMTLFileName = BaseName + '.mtl'

Instance = _ngp.PlantInstance(SourceFileName)

OBJFile = open(TargetOBJFileName,'wt')

OBJFile.write('o plant\n')
OBJFile.write('mtllib %s\n' % (TargetMTLFileName))

VertexIndexOffset   = 1
NormalIndexOffset   = 1
TexCoordIndexOffset = 1

for GroupIndex in range(Instance.GetGroupCount()):
    Group    = Instance.GetGroup(GroupIndex)
    Material = Group.GetMaterial()

    OBJFile.write('usemtl pmat%u\n' % (GroupIndex))

    if Material.TexNames[_ngp.TEX_DIFFUSE] is None:
        OBJFile.write('usemap off\n')
    else:
        OBJFile.write('usemap %s\n' % (os.path.join(TexturePathPrefix,Material.TexNames[_ngp.TEX_DIFFUSE])))

    Buffer = Group.GetVAttrBuffer(_ngp.ATTR_VERTEX)

    VertexIndexStep = len(Buffer)

    for (x,y,z) in Buffer:
        OBJFile.write('v %f %f %f\n' % (x,y,z))

    Buffer = Group.GetVAttrBuffer(_ngp.ATTR_NORMAL)

    NormalIndexStep = len(Buffer)

    for (x,y,z) in Buffer:
        OBJFile.write('vn %f %f %f\n' % (x,y,z))

    Buffer = Group.GetVAttrBuffer(_ngp.ATTR_TEXCOORD0)

    TexCoordIndexStep = len(Buffer)

    for (u,v) in Buffer:
        OBJFile.write('vt %f %f\n' % (u,v))

    del Buffer

    VertexIndexBuffer   = Group.GetVAttrIndexBuffer(_ngp.ATTR_VERTEX,1,VertexIndexOffset)
    NormalIndexBuffer   = Group.GetVAttrIndexBuffer(_ngp.ATTR_NORMAL,1,NormalIndexOffset)
    TexCoordIndexBuffer = Group.GetVAttrIndexBuffer(_ngp.ATTR_TEXCOORD0,1,TexCoordIndexOffset)

    for PrimitiveIndex in range(len(VertexIndexBuffer)):
        vi = VertexIndexBuffer[PrimitiveIndex]
        ni = NormalIndexBuffer[PrimitiveIndex]
        ti = TexCoordIndexBuffer[PrimitiveIndex]

        if   Group.GetPrimitiveType(PrimitiveIndex) == _ngp.QUAD:
            OBJFile.write('f %u/%u/%u %u/%u/%u %u/%u/%u %u/%u/%u\n' %\
                          (vi[0],ti[0],ni[0],
                           vi[1],ti[1],ni[1],
                           vi[2],ti[2],ni[2],
                           vi[3],ti[3],ni[3]))
        elif Group.GetPrimitiveType(PrimitiveIndex) == _ngp.TRIANGLE:
            OBJFile.write('f %u/%u/%u %u/%u/%u %u/%u/%u\n' %\
                          (vi[0],ti[0],ni[0],
                           vi[1],ti[1],ni[1],
                           vi[2],ti[2],ni[2]))
        else:
            raise RuntimeError('unknown primitive type')

    VertexIndexOffset   += VertexIndexStep
    NormalIndexOffset   += NormalIndexStep
    TexCoordIndexOffset += TexCoordIndexStep

OBJFile.close()

MTLFile = open(TargetMTLFileName,'wt')

for GroupIndex in range(Instance.GetGroupCount()):
    Group    = Instance.GetGroup(GroupIndex)
    Material = Group.GetMaterial()

    MTLFile.write('newmtl pmat%u\n' % (GroupIndex))

    R,G,B = Material.Color

    MTLFile.write('Kd %f %f %f\n' % (R,G,B))

    if Material.TexNames[_ngp.TEX_DIFFUSE] is not None:
        MTLFile.write('map_Kd %s\n' % (os.path.join(TexturePathPrefix,Material.TexNames[_ngp.TEX_DIFFUSE])))

MTLFile.close()

