#include "OpenPfb.h"

using std::auto_ptr;
using std::string;

const char *OpenPfb_GetVersion()
{
    return "OpenPfb v0.2b (c)2006, Jean-Christophe HOELT - CNRS-IRIT, Toulouse, France";
}

namespace openpfb
{
FILE *debugfile = NULL;
static bool  needBswap = false;

//
// UTILS
//

struct PfbString
{
    uint32_t length;
    char    *str; // strlen(str) = length

    PfbString();
    ~PfbString();
};

struct PfbNodeEnd
{
    uint32_t  mask[4];   // = 0xffffffff x 4
    int32_t   data0[4]; // = 0 -1
    PfbString name;
};
            

static void bswap(uint32_t *array, uint32_t size = 1)
{
    if (!needBswap) return;

    for (unsigned i=0; i<size; ++i)
    {
        union bswap_union
        {
            uint32_t u32;
            uint8_t  u8[4];
        };
        bswap_union input;
        bswap_union output;
        
        input.u32 = array[i];
        
        output.u8[0] = input.u8[3];
        output.u8[1] = input.u8[2];
        output.u8[2] = input.u8[1];
        output.u8[3] = input.u8[0];

        array[i] = output.u32;
    }
}
static void bswap(int32_t *array, uint32_t size = 1) { bswap((uint32_t*)array, size); }
static void bswap(float *array, uint32_t size = 1) { bswap((uint32_t*)array, size); }

// STRING

PfbString::PfbString() : length(0), str(NULL) {}

PfbString::~PfbString()
{
    if (str) delete[] str;
}

// CHILDS

void PfbChilds::setNumChildren(uint32_t num)
{
    numChildren = num;
    if (num > 0)
        childs = new uint32_t[num];
}

PfbChilds::PfbChilds() : numChildren(0), childs(NULL) {}

PfbChilds::~PfbChilds()
{
    if (childs) delete[] childs;
}

// LOD

void PfbNodeLOD::setNumRanges(uint32_t num) {
    numRanges = num;
    ranges = new float[num+1];
}

PfbNodeLOD::PfbNodeLOD() : ranges(NULL) {}

PfbNodeLOD::~PfbNodeLOD()
{
    if (ranges) delete[] ranges;
}

// GEODE

PfbNodeGeode::PfbNodeGeode() : numGeosets(0), geosets(NULL) {}
PfbNodeGeode::~PfbNodeGeode()
{
    if (geosets) delete[] geosets;
}

void PfbNodeGeode::setNumGeosets(uint32_t num)
{
    numGeosets = num;
    geosets = new uint32_t[num];
}

// NODE

PfbNode::PfbNode() : type(0), name(NULL)
{}

PfbNode::~PfbNode()
{
    switch (type)
    {
        case 2: delete data.geode; break;
        case 5: delete data.group; break;
        case 6: delete data.scs; break;
        case 7: delete data.dcs; break;
        case 11: delete data.lod; break;
    }
    if (name) delete[] name;
}

void PfbNode::setType(uint32_t type)
{
    this->type = type;
    switch (type)
    {
        case 2:  // Geode
            data.geode = new PfbNodeGeode(); break;
        case 5:  // Group
            data.group = new PfbNodeGroup(); break;
        case 6:  // SCS
            data.scs   = new PfbNodeSCS(); break;
        case 7:  // SCS
            data.scs   = new PfbNodeDCS(); break;
        case 11: // LOD
            data.lod   = new PfbNodeLOD(); break;
        default:
            return;
    }
}

void PfbNode::setName(const char *str, uint32_t strlength)
{
    if (name) delete[] name;
    name = new char[strlength+1];
    if (strlength == 0) name[0] = 0;
    if (str == NULL)    name[0] = 0;
    strcpy(name, str);
}

// TREE
   
PfbTree::PfbTree()
    : lengthList(NULL)
    , vertexList(NULL)
    , colorList(NULL)
    , normalList(NULL)
    , texcoordList(NULL)
    , materials(NULL)
    , textures(NULL)
    , geostates(NULL)
    , geosets(NULL)
    , nodes(NULL)
    , numLengthList(0)
    , numVertexList(0)
    , numColorList(0)
    , numNormalList(0)
    , numTexcoordList(0)
    , numMaterials(0)
    , numTextures(0)
    , numGeoStates(0)
    , numGeoSets(0)
    , numNodes(0)
{}

PfbTree::~PfbTree()
{
    if (lengthList)   delete[] lengthList;
    if (vertexList)   delete[] vertexList;
    if (colorList)    delete[] colorList;
    if (normalList)   delete[] normalList;
    if (texcoordList) delete[] texcoordList;
    if (materials)    delete[] materials;
    if (textures)     delete[] textures;
    if (geostates)    delete[] geostates;
    if (geosets)      delete[] geosets;
    if (nodes)        delete[] nodes;
}
void PfbTree::createLengthLists(unsigned num)
{
    lengthList = new PfbLengthList[num];
    numLengthList = num;
}
void PfbTree::createTexcoordLists(unsigned num)
{
    texcoordList = new PfbTexcoordList[num];
    numTexcoordList = num;
}
void PfbTree::createNormalLists(unsigned num)
{
    normalList = new PfbNormalList[num];
    numNormalList = num;
}
void PfbTree::createVertexLists(unsigned num)
{
    vertexList = new PfbVertexList[num];
    numVertexList = num;
}
void PfbTree::createColorLists(unsigned num)
{
    colorList  = new PfbColorList[num];
    numColorList  = num;
}

void PfbTree::createMaterials(unsigned num)
{
    materials = new PfbMaterial[num];
    numMaterials = num;
}

void PfbTree::createTextures(unsigned num)
{
    textures = new PfbTexture[num];
    numTextures = num;
}

void PfbTree::createGeoStates(unsigned num)
{
    geostates = new PfbGeoState[num];
    numGeoStates = num;
}

void PfbTree::createGeoSets(unsigned num)
{
    geosets = new PfbGeoSet[num];
    numGeoSets = num;
}

void PfbTree::createNodes(unsigned num)
{
    nodes = new PfbNode[num];
    numNodes = num;
}
            
/// PFB Loader
///
/// @author Jean-Christophe Hoelt
PfbFile::PfbFile(const string &name) : name(name), error(NULL)
{
    f = fopen(name.c_str(), "r");
    if (f == NULL) error = "Could not open file";

    if (error)
        printf("hidra::PfbLoader: could not open file\n");
}

PfbFile::~PfbFile()
{
    if (f) fclose(f);
}

bool PfbFile::loadFailed() const      { return error != NULL; }
const char *PfbFile::getError() const { return error; }

//
// HEADER /* {{{ */
//

PfbFile::PfbHeader PfbFile::readHeader()
{
    if (error) return PfbHeader();

    PfbHeader header;
    fread(&header, sizeof(PfbHeader), 1, f);
    if (header.magic == 0x00ce0adb) {
        needBswap = true;
        bswap(&header.magic);
    }
    else if (header.magic == 0xdb0ace00) {
        needBswap = false;
    }
    else {
        printf("hidra::PfbLoader: Unsupported pfb.\n");
        error = "Unsupported pfb";
    }
    return header;
}
/* }}} */

//
// LENGTH /* {{{ */
//

void PfbFile::readLengthList(PfbLengthList &list)
{
    struct {
        uint32_t size;
        int32_t  unknown1;
        int32_t  unknown2;
    } info;
    fread(&info, sizeof(info), 1, f);
    bswap(&info.size, 3);

    if (debugfile) fprintf(debugfile, "(%u)\n", info.size);
    list.allocate(info.size);

    fread(list.get(0), 4, info.size, f);
    bswap(list.get(0), info.size);
}

void PfbFile::readLengthLists()
{
    struct {
        uint32_t numLists;
        uint32_t totalSize;
    } info;

    fread(&info, sizeof(info), 1, f);
    bswap(&info.numLists, 2);

    if (debugfile) fprintf(debugfile, "hidra::PfbLoader: %u length lists\n", info.numLists);
    tree->createLengthLists(info.numLists);

    for (unsigned i=0; i<info.numLists; ++i) {
        if (debugfile) fprintf(debugfile, "hidra::PfbLoader:   list[%u] ", i);
        readLengthList(tree->getLengthList(i));
    }
}
/* }}} */

//
// VERTEX /* {{{ */
//

void PfbFile::readVertexList(PfbVertexList &list)
{
    struct {
        uint32_t size;
        int32_t  unknown1;
        int32_t  unknown2;
    } info;
    fread(&info, sizeof(info), 1, f);
    bswap(&info.size, 3);

    if (debugfile) fprintf(debugfile, "(%u)\n", info.size);
    list.allocate(info.size);

    fread(list.get(0), 4*3, info.size, f);
    bswap(list.get(0), 3 * info.size);
}

void PfbFile::readVertexLists()
{
    struct {
        uint32_t numLists;
        uint32_t totalSize;
    } info;

    fread(&info, sizeof(info), 1, f);
    bswap(&info.numLists, 2);

    if (debugfile) fprintf(debugfile, "hidra::PfbLoader: %u vertex lists\n", info.numLists);
    tree->createVertexLists(info.numLists);

    for (unsigned i=0; i<info.numLists; ++i) {
        if (debugfile) fprintf(debugfile, "hidra::PfbLoader:   list[%u] ", i);
        readVertexList(tree->getVertexList(i));
    }
}
/* }}} */

//
// COLOR  /* {{{ */
//

void PfbFile::readColorList(PfbColorList &list)
{
    struct {
        uint32_t size;
        int32_t  unknown1;
        int32_t  unknown2;
    } info;
    fread(&info, sizeof(info), 1, f);
    bswap(&info.size, 3);

    if (debugfile) fprintf(debugfile, "(%u)\n", info.size);
    list.allocate(info.size);

    fread(list.get(0), 4*4, info.size, f);
    bswap(list.get(0), 4 * info.size);
}

void PfbFile::readColorLists()
{
    struct {
        uint32_t numLists;
        uint32_t totalSize;
    } info;

    fread(&info, sizeof(info), 1, f);
    bswap(&info.numLists, 2);

    if (debugfile) fprintf(debugfile, "hidra::PfbLoader: %u color lists\n", info.numLists);
    tree->createColorLists(info.numLists);

    for (unsigned i=0; i<info.numLists; ++i) {
        if (debugfile) fprintf(debugfile, "hidra::PfbLoader:   list[%u] ", i);
        readColorList(tree->getColorList(i));
    }
}
/* }}} */

//
// NORMAL LIST /* {{{ */
//

void PfbFile::readNormalList(PfbNormalList &list)
{
    struct {
        uint32_t size;
        int32_t  unknown1;
        int32_t  unknown2;
    } info;
    fread(&info, sizeof(info), 1, f);
    bswap(&info.size, 3);

    if (debugfile) fprintf(debugfile, "(%u)\n", info.size);
    list.allocate(info.size);

    fread(list.get(0), 4*3, info.size, f);
    bswap(list.get(0), 3 * info.size);
}

void PfbFile::readNormalLists()
{
    struct {
        uint32_t numLists;
        uint32_t totalSize;
    } info;

    fread(&info, sizeof(info), 1, f);
    bswap(&info.numLists, 2);

    if (debugfile) fprintf(debugfile, "hidra::PfbLoader: %u normal lists\n", info.numLists);
    tree->createNormalLists(info.numLists);

    for (unsigned i=0; i<info.numLists; ++i) {
        if (debugfile) fprintf(debugfile, "hidra::PfbLoader:   list[%u] ", i);
        readNormalList(tree->getNormalList(i));
    }
}
/* }}} */

//
// TEXCOORD LIST /* {{{ */
//

void PfbFile::readTexcoordList(PfbTexcoordList &list)
{
    struct {
        uint32_t size;
        int32_t  unknown1;
        int32_t  unknown2;
    } info;
    fread(&info, sizeof(info), 1, f);
    bswap(&info.size, 3);

    if (debugfile) fprintf(debugfile, "(%u)\n", info.size);
    list.allocate(info.size);

    fread(list.get(0), 4*2, info.size, f);
    bswap(list.get(0), 2 * info.size);
}

void PfbFile::readTexcoordLists()
{
    struct {
        uint32_t numLists;
        uint32_t totalSize;
    } info;

    fread(&info, sizeof(info), 1, f);
    bswap(&info.numLists, 2);

    if (debugfile) fprintf(debugfile, "hidra::PfbLoader: %u texcoord lists\n", info.numLists);
    tree->createTexcoordLists(info.numLists);

    for (unsigned i=0; i<info.numLists; ++i) {
        if (debugfile) fprintf(debugfile, "hidra::PfbLoader:   list[%u] ", i);
        readTexcoordList(tree->getTexcoordList(i));
    }
}
/* }}} */

//
// MATERIAL /* {{{ */
//

void PfbFile::readMaterial(PfbMaterial &material)
{
    uint32_t materialType;
    fread(&materialType, sizeof(materialType), 1, f);
    bswap(&materialType);

    switch (materialType)
    {
        case 1:
        case 2:
            break;
        default:
            printf("hidra::PfbLoader: Unsupported material type: %u\n",materialType);
            error = "Unsupported material type";
            return;
    }
    material.type = materialType;

    fread(((uint32_t*)&material) + 1, sizeof(material) - 4, 1, f);
    bswap(((uint32_t*)&material) + 1, (sizeof(material)/4) - 1);
}

void PfbFile::readMaterials()
{
    struct {
        uint32_t numMaterials;
        uint32_t totalSize;
    } info;

    fread(&info, sizeof(info), 1, f);
    bswap(&info.numMaterials, 2);

    if (debugfile) fprintf(debugfile, "hidra::PfbLoader: %u materials\n", info.numMaterials);
    tree->createMaterials(info.numMaterials);

    for (unsigned i=0; i<info.numMaterials; ++i) {
        if (debugfile) fprintf(debugfile, "hidra::PfbLoader:   material[%u]\n", i);
        readMaterial(tree->getMaterial(i));
        if (error) return;
    }
}
/* }}} */

//
// TEXTURE /* {{{ */
//
            
void PfbFile::readTexture(PfbTexture &texture)
{
    // Read FileName
    PfbString str;
    readString(str);
    if (error) return;
    texture.fileName = new char[str.length+1];
    strncpy(texture.fileName, str.str, str.length+1);

    if (debugfile) fprintf(debugfile, "hidra::PfbLoader:   textureName=%s\n", texture.fileName);

    // Read Remaining...
    //uint32_t *remainingRead = (uint32_t*)&texture.five_1_a;
    //uint32_t  remainingSize2 =  sizeof(PfbTexture) - sizeof(const char *);
    uint32_t  remainingSize = 228;
    // uint32_t  remainingSize = 232;
    //fprintf(stderr, "r2=%d\n", remainingSize2);
    //fread(remainingRead, remainingSize, 1, f);
    //bswap(remainingRead, remainingSize/4);
    fseek(f,remainingSize,SEEK_CUR);

    // return 228+remainingSize+str.length+4;
}

void PfbFile::readTextures()
{
    struct {
        uint32_t numTextures;
        uint32_t totalSize;
    } info;

    fread(&info, sizeof(info), 1, f);
    bswap(&info.numTextures, 2);

    if (debugfile) fprintf(debugfile, "hidra::PfbLoader: %u textures\n", info.numTextures);
    tree->createTextures(info.numTextures);

    long start=ftell(f);

    for (unsigned i=0; i<info.numTextures; ++i) {
        if (debugfile) fprintf(debugfile, "hidra::PfbLoader:   texture[%u]\n", i);
        readTexture(tree->getTexture(i));
        if (error) return;
    }

    long end=ftell(f);
    if (info.totalSize > end-start) {
        long offset = info.totalSize+start-end;
        fseek(f,offset,SEEK_CUR);
    }
}
 /* }}} */

//
// GEOSTATE /* {{{ */
//

void PfbFile::readGeoState(PfbGeoState &geostate)
{
    int32_t numValues;
    fread(&numValues, sizeof(numValues), 1, f);
    bswap(&numValues, 1);

    geostate.setNumValues(numValues);
    int32_t key;
    int32_t nextkey = 0;
    
    while (true)
    {
        if (nextkey == 0) {
            fread(&key, sizeof(key), 1, f);
            bswap(&key);
        }
        else {
            key = nextkey;
            nextkey = 0;
        }
        if (key == -1) return;

        int32_t value;
        fread(&value, sizeof(value), 1, f);
        bswap(&value);
        geostate.setValue(key,value);
        
        if ((key == 6) || (key == 13)) {
            int32_t one;
            fread(&one, sizeof(one), 1, f);
            bswap(&one);
            if (one == 1)
                fseek(f,8,SEEK_CUR);
            else
                nextkey = one;
        }
        
        if ((key == 17) || (key == 18) || (key == 25)) {
            int32_t one;
            fread(&one, sizeof(one), 1, f);
            bswap(&one);
            if (one == -1) {
                fread(&one, sizeof(one), 1, f);
                bswap(&one);
                if (one != -1) {
                    fseek(f,-4,SEEK_CUR);
                    return;
                }
                fread(&one, sizeof(one), 1, f);
                bswap(&one);
            }
            else
                nextkey = one;
        }
    }
}

void PfbFile::readGeoStates()
{
    struct {
        uint32_t numStates;
        uint32_t totalSize;
    } info;

    fread(&info, sizeof(info), 1, f);
    bswap(&info.numStates, 2);

    tree->createGeoStates(info.numStates);
    if (debugfile) fprintf(debugfile, "hidra::PfbLoader: %u geostates\n", info.numStates);

    for (unsigned i=0; i<info.numStates; ++i)
    {
        if (debugfile) fprintf(debugfile, "hidra::PfbLoader:   geostates[%u] ", i);
        readGeoState(tree->getGeoState(i));
        if (error) return;
    }
}
/* }}} */

//
// GEOSET /* {{{ */
//

void PfbFile::readGeoSets()
{
    struct {
        uint32_t numSets;
        uint32_t totalSize;
    } info;

    fread(&info, sizeof(info), 1, f);
    bswap(&info.numSets, 2);
    uint32_t sizePerSet  = info.totalSize / info.numSets;
    uint32_t padding     = sizePerSet - sizeof(PfbGeoSet);

    tree->createGeoSets(info.numSets);
    if (debugfile) fprintf(debugfile, "hidra::PfbLoader: %u geosets\n", info.numSets);

    for (unsigned i=0; i<info.numSets; ++i)
    {
        if (debugfile) fprintf(debugfile, "hidra::PfbLoader:   geoset[%u] ", i);
        fread(&tree->getGeoSet(i), sizeof(PfbGeoSet), 1, f);
        bswap((uint32_t*)&tree->getGeoSet(i), sizeof(PfbGeoSet)/4);
        fseek(f, padding, SEEK_CUR);
        if (debugfile) fprintf(debugfile, "(%u,%u)\n", tree->getGeoSet(i).stripType, tree->getGeoSet(i).numStrip);
    }
} /* }}} */

//
// NODES /* {{{ */
//
void PfbFile::readString(PfbString &pstr)
{
    fread(&pstr.length, sizeof(pstr.length), 1, f);
    bswap(&pstr.length);

    if (pstr.length == 0xffffffff) pstr.length=0;
    if (pstr.length > 0x1000) {
        pstr.length=0;
        error = "Invalid string length";
        return;
    }
    pstr.str = new char[pstr.length+1];
    pstr.str[pstr.length] = 0;

    if (pstr.length > 0)
        fread(pstr.str, pstr.length, 1, f);
}

void PfbFile::readNodeEnd(PfbNodeEnd &nodeEnd, long namePosition)
{
    fread(&nodeEnd.mask[0],  4, 4, f);
    fread(&nodeEnd.data0[0], 4, 4, f);
    fseek(f, namePosition, SEEK_SET);
    readString(nodeEnd.name);
    if (debugfile) fprintf(debugfile, "hidra::PfbLoader:   name=%s (%u)\n ", nodeEnd.name.str, nodeEnd.name.length);

    bswap(&nodeEnd.mask[0], 4);
    bswap(&nodeEnd.data0[0], 4);
}

void PfbFile::readChilds(PfbChilds &childs)
{
    uint32_t numChildren;
    fread(&numChildren, 4, 1, f);
    bswap(&numChildren);

    childs.setNumChildren(numChildren);
    fread(&childs.childs[0], 4, numChildren, f);
    bswap(&childs.childs[0], numChildren);
}

void PfbFile::readNodeLOD(PfbNodeLOD &lod)
{
    uint32_t numRanges;
    fread(&numRanges, sizeof(numRanges), 1, f);
    bswap(&numRanges);

    lod.setNumRanges(numRanges);
    fread(lod.getRanges(0), 4, numRanges+1, f);
    bswap(lod.getRanges(0),    numRanges+1);

    float *ones = new float[numRanges+1];
    fread(&ones[0],   4, numRanges+1, f);
    bswap(&ones[0],      numRanges+1);
    delete[] ones;
    
    fread(lod.getCenter(), 4, 3, f);
    bswap(lod.getCenter(), 3);

    int32_t minusOne[2];
    fread(&minusOne[0], 4, 2, f);
    bswap(&minusOne[0], 2);

    readChilds(lod.getChilds());
}

void PfbFile::readNodeGeode(PfbNodeGeode &geode)
{
    uint32_t numGeosets;
    fread(&numGeosets, 4, 1, f);
    bswap(&numGeosets);

    geode.setNumGeosets(numGeosets);
    fread(geode.getGeosets(), 4, numGeosets, f);
    bswap(geode.getGeosets(), numGeosets);
}

void PfbFile::readNodeSCS(PfbNodeSCS &scs)
{
    fread(scs.getMatrix(), 4, 16, f);
    bswap(scs.getMatrix(), 16);
    readChilds(scs.getChilds());
}

void PfbFile::readNodeDCS(PfbNodeDCS &dcs)
{
    uint32_t mask;
    fread(&mask, 4, 1, f);
    bswap(&mask);
    
    fread(dcs.getMatrix(), 4, 16, f);
    bswap(dcs.getMatrix(), 16);
    readChilds(dcs.getChilds());
}

void PfbFile::readNodeGroup(PfbNodeGroup &group)
{
    readChilds(group.getChilds());
}

void PfbFile::readNode(PfbNode &node)
{
    uint32_t nodeSize;
    fread(&nodeSize, sizeof(nodeSize), 1, f);
    bswap(&nodeSize);

    long pos = ftell(f);

    uint32_t type;
    fread(&type, sizeof(type), 1, f);
    bswap(&type);
    node.setType(type);

    switch (type)
    {
        case 2:  // Geode
            if (debugfile) fprintf(debugfile, "hidra::PfbLoader:   node_Geode\n");
            readNodeGeode(*node.asGeode());
            break;
        case 5:  // Group
            if (debugfile) fprintf(debugfile, "hidra::PfbLoader:   node_Group\n");
            readNodeGroup(*node.asGroup());
            break;
        case 6:  // SCS
            if (debugfile) fprintf(debugfile, "hidra::PfbLoader:   node_SCS\n");
            readNodeSCS(*node.asSCS());
            break;
        case 7:  // DCS
            if (debugfile) fprintf(debugfile, "hidra::PfbLoader:   node_DCS\n");
            readNodeDCS(*node.asDCS());
            break;
        case 11: // LOD
            if (debugfile) fprintf(debugfile, "hidra::PfbLoader:   node_LOD\n");
            readNodeLOD(*node.asLOD());
            break;
        default:
            printf("hidra::PfbLoader: Unsupported node type: %u\n", type);
            error = "Unsupported node type";
            return;
    }

    PfbNodeEnd nodeEnd;
    readNodeEnd(nodeEnd, pos + nodeSize*4);
    node.setName(nodeEnd.name.str, nodeEnd.name.length);
}

void PfbFile::readNodes()
{
    struct {
        uint32_t numNodes;
        uint32_t totalSize;   // 257 | 269
    } info;

    fread(&info, sizeof(info), 1, f);
    bswap(&info.numNodes, 2);

    // long pos = ftell(f);

    if (debugfile) fprintf(debugfile, "hidra::PfbLoader: %u nodes\n", info.numNodes);
    tree->createNodes(info.numNodes);

    for (unsigned i=0; i<info.numNodes; ++i) {
        readNode(tree->getNode(i));
        if (error) return;
    }

    // fseek(f, pos + info.totalSize, SEEK_SET);
}
 /* }}} */

//
// SKIP BLOCK
//
void PfbFile::skipBlock()
{
    struct {
        uint32_t num;
        uint32_t totalSize;
    } info;
    fread(&info, sizeof(info), 1, f);
    bswap(&info.totalSize, 1);
    fseek(f, info.totalSize, SEEK_CUR);
}

//
// DATA HEADER
//

void PfbFile::readNext()
{
    if (error) return;

    uint32_t type;
    fread(&type, 4, 1, f);
    if (feof(f)) return;
    bswap(&type);

    switch(type)
    {
        case 0: // Materials
            readMaterials();
            break;
        case 1: // Texture
            readTextures();
            break;
        case 2: // Texenv?
            if (debugfile) fprintf(debugfile, "hidra::PfbLoader: WARNING, Ignoring TexEnv\n");
            skipBlock();
            break;
        case 3: // GeoStates
            readGeoStates();
            break;
        case 4: // Length lists
            readLengthLists();
            break;
        case 5: // Vertex lists
            readVertexLists();
            break;
        case 6: // Color lists
            readColorLists();
            break;
        case 7: // Normal lists
            readNormalLists();
            break;
        case 8: // Texcoord lists
            readTexcoordLists();
            break;
        case 10: // GeoSets
            readGeoSets();
            break;
        case 17: // TexGens
            skipBlock();
            if (debugfile) fprintf(debugfile, "hidra::PfbLoader: WARNING, Ignoring TexGens\n");
            break;
        case 18: // Light models
            skipBlock();
            if (debugfile) fprintf(debugfile, "hidra::PfbLoader: WARNING, Ignoring LightModels\n");
            break;
        case 27: // Images (TODO)
            skipBlock();
            if (debugfile) fprintf(debugfile, "hidra::PfbLoader: WARNING, Ignoring Images\n");
            break;
        case 12: // Nodes
            readNodes();
            break;
        default:
            printf("hidra::PfbLoader: Unknown block ID: 0x%08x (%u)\n", type, type);
            error = "Unknown block ID";
    }
}

auto_ptr<PfbTree> PfbFile::load()
{
    if (error) return auto_ptr<PfbTree>(NULL);
    // debugfile = stderr;

    tree = new PfbTree();
    readHeader();

    while (!error) {
        readNext();
        if (feof(f)) break;
    }

    return auto_ptr<PfbTree>(tree);
}
}
