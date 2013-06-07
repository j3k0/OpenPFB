#ifndef _OPENPFB_H
#define _OPENPFB_H

#include <stdint.h>
#include <cstdlib>
#include <cstdio>

#include <memory>
#include <string>

namespace openpfb
{
    extern FILE *debugfile; // default is NULL, change to activate debugging

    template <typename T, unsigned N>
    class PfbList
    {
        public:
            PfbList() : size(0), array(NULL) {}
            ~PfbList() {
                if (array) delete[] array;
            }
            void allocate(unsigned size) {
                this->size = size;
                array = new T[size * N];
            }
            T *get(unsigned i) {
                return array + i * N;
            }
            const T *get(unsigned i) const {
                return array + i * N;
            }
            unsigned getSize() const { return size; }

            /// Ensure the array will not be destroyed in destructor
            void release() {
                array = NULL;
                size = 0;
            }
        private:
            unsigned size;
            T *array;
    };

    class PfbVertexList : public PfbList<float,3> {};
    class PfbLengthList : public PfbList<uint32_t,1> {};
    class PfbColorList  : public PfbList<float,4> {};
    class PfbNormalList : public PfbList<float,3> {};
    class PfbTexcoordList : public PfbList<float,2> {};
    
    struct PfbMaterial
    {
        uint32_t type;  // = 1 | 2
        float    data2; // = 1 | 1
        float    shininess; // 16 | 32
        float    specular[3]; // diffuse ? (0.6 0.6 0.6)
        float    diffuse[3]; // ? (0.8 0.8 0.8)
        float    diffuse_bis[3]; // ? (0.72 0.72 0.72)
        float    ambiant[3]; // uint/float ? (0 0 0) | 0 0 0
        int32_t  data8[2]; // uint ? (3 4) | (3 1)
        int32_t  data9;    // -1 | -1
    };

    struct PfbTexture
    {
        char *fileName;
        int five_1_a[5];
        int four_hexa_a[4];
        int three_1_a[3];
        int four_0_a[4];
        int one_1_a;
        float nine_values_a[9];
        float nine_values_b[9];
        int zero_minus_one_a[2];
        int three_1_b[3];
        int two_0_a[2];
        int two_0_b[2];
        int two_minus_one_a[2];
        int five_minus_one_a[2];
        int one_minus_one_a;
        float zero_minus_one_b[2];
        int one_zero_a;
        int one_minus_one_b;
        int one_zero_b;
    };

#define PFBSTATE_TRANSPARENCY  1
#define PFBSTATE_ALPHAFUNC     4
#define PFBSTATE_ENLIGHTING    5
#define PFBSTATE_ENTEXTURE     6
#define PFBSTATE_CULLFACE      8
#define PFBSTATE_ENCOLORTABLE  10
#define PFBSTATE_ENLPOINTSTATE 12
#define PFBSTATE_FRONTMTL      15
#define PFBSTATE_TEXTURE       17
#define PFBSTATE_TEXENV        18

    class PfbGeoState
    {
        public:
        PfbGeoState() : numValues(0), values(NULL) {}
        ~PfbGeoState() { if (values) delete[]values; }

        void setNumValues(unsigned num) {
            numValues=num;
            values=new int32_t[num];
            for (unsigned i=0; i<num; ++i)
                values[i] = -1;
        }
        int32_t getValue(int i) const {
            if ((i-1)<numValues) return values[i-1]; else return 0;
        }
        void setValue(int i, int32_t value) {
            if ((i-1)<numValues) values[i-1] = value;
        }

        private:
        int32_t  numValues;
        int32_t *values;
    };

    struct PfbGeoSet
    {
        uint32_t stripType; // 1 = normal, 7 = normal, 8 = nostrip
        uint32_t numStrip;
        int32_t  lengthListId;

        // ignore the following
        int32_t data1[3];
        int32_t data2[3];
        int32_t data3[3];
        int32_t data4[3];
        int32_t data5[3];

        // Id of the geostate
        int32_t geostateId;
        int32_t data6b;
        int32_t data7[2];
        int32_t data8;
        uint32_t mask;
        int32_t data9;
        int32_t num; // 2
        float vec[6];
    };

    struct PfbChilds
    {
        uint32_t numChildren;
        uint32_t *childs; // (numChildren) uint

        PfbChilds();
        ~PfbChilds();
        
        void     setNumChildren(uint32_t num);
        uint32_t getNumChildren() const     { return numChildren; }
        uint32_t getChild(uint32_t i) const { return childs[i]; }
    };
    
    class PfbNodeLOD
    {
        private:
            uint32_t  numRanges;
            float    *ranges;      // (numRanges+1) float
            float     center[3];   // center of geometry (?)
            PfbChilds childs;

        public:
            PfbNodeLOD();
            ~PfbNodeLOD();

            void setNumRanges(uint32_t num);

            /// Return the number of defined ranges
            uint32_t     getNumRanges() const        { return numRanges;  }

            /// Returns range for child i, as two floats.
            /// ret[0] = min, ret[1] = max
            float       *getRanges(uint32_t i)       { return ranges + i; }
            /// Returns range for child i, as two floats.
            /// ret[0] = min, ret[1] = max
            const float *getRanges(uint32_t i) const { return ranges + i; }

            /// Return center of LOD (float[3])
            float *getCenter()             { return &center[0]; }
            /// Return center of LOD (float[3])
            const float *getCenter() const { return &center[0]; }

            /// Return node childs id
            PfbChilds &getChilds()             { return childs; }
            /// Return node childs id (read-only)
            const PfbChilds &getChilds() const { return childs; }
    };

    class PfbNodeGeode
    {
        private:
            uint32_t  numGeosets;
            uint32_t *geosets;

        public:
            PfbNodeGeode();
            ~PfbNodeGeode();

            void setNumGeosets(uint32_t num);
            uint32_t getNumGeosets() const     { return numGeosets; }

            uint32_t *getGeosets()             { return geosets; }
            const uint32_t *getGeosets() const { return geosets; }
    };

    class PfbNodeTransform
    {
        private:
            float     matrix[16];
            PfbChilds childs;

        public:
            /// Return node childs id
            PfbChilds &getChilds()             { return childs; }
            /// Return node childs id (read-only)
            const PfbChilds &getChilds() const { return childs; }

            /// Return transformation matrix (float[4][4])
            float *getMatrix()                 { return &matrix[0]; }
            /// Return transformation matrix (float[4][4]) read-only
            const float *getMatrix() const     { return &matrix[0]; }
    };

    typedef PfbNodeTransform PfbNodeSCS;
    typedef PfbNodeTransform PfbNodeDCS;
    
    class PfbNodeGroup
    {
        private:
            PfbChilds childs;

        public:
            /// Return node childs id
            PfbChilds &getChilds()             { return childs; }
            /// Return node childs id (read-only)
            const PfbChilds &getChilds() const { return childs; }
    };

    class PfbNode
    {
        private:
            uint32_t type;
            union {
                PfbNodeGeode *geode; // type 2
                PfbNodeGroup *group; // type 5
                PfbNodeSCS   *scs;   // type 6
                PfbNodeDCS   *dcs;   // type 7
                PfbNodeLOD   *lod;   // type 11
            } data;
            char *name;

        public:
            PfbNode();
            ~PfbNode();

            PfbNodeGeode *asGeode() { return (type==2)  ? data.geode : NULL; }
            PfbNodeGroup *asGroup() { return (type==5)  ? data.group : NULL; }
            PfbNodeSCS   *asSCS()   { return (type==6)  ? data.scs   : NULL; }
            PfbNodeDCS   *asDCS()   { return (type==7)  ? data.dcs   : NULL; }
            PfbNodeLOD   *asLOD()   { return (type==11) ? data.lod   : NULL; }

            void setType(uint32_t type);
            uint32_t getType() const { return type; }

            void setName(const char *str, uint32_t strlength);
            const char *getName() const { return name; }
    };
   
    /// @class PfbTree
    ///
    /// @brief Scene graph
    class PfbTree
    {
        public:
            PfbTree();
            ~PfbTree();

            /// @name Accessors
            /// @{

            PfbNode         &getRootNode()                 { return getNode(0); }
            PfbNode         &getNode(unsigned i)           { return nodes[i];   }

            PfbLengthList   &getLengthList(unsigned i)     { return lengthList[i];   }
            PfbVertexList   &getVertexList(unsigned i)     { return vertexList[i];   }
            PfbNormalList   &getNormalList(unsigned i)     { return normalList[i];   }
            PfbTexcoordList &getTexcoordList(unsigned i)   { return texcoordList[i]; }
            PfbColorList    &getColorList(unsigned i)      { return colorList[i];    }
            PfbMaterial     &getMaterial(unsigned i)       { return materials[i];    }
            PfbTexture      &getTexture(unsigned i)        { return textures[i];     }
            PfbGeoState     &getGeoState(unsigned i)       { return geostates[i];    }
            PfbGeoSet       &getGeoSet(unsigned i)         { return geosets[i];      }

            bool haveLengthList() const   { return lengthList   != NULL; }
            bool haveVertexList() const   { return vertexList   != NULL; }
            bool haveColorList() const    { return colorList    != NULL; }
            bool haveNormalList() const   { return normalList   != NULL; }
            bool haveTexcoordList() const { return texcoordList != NULL; }
            bool haveMaterials() const    { return materials    != NULL; }
            bool haveTextures() const     { return textures     != NULL; }
            
            unsigned getNumLengthList() const   { return numLengthList; }
            unsigned getNumVertexList() const   { return numVertexList; }
            unsigned getNumColorList() const    { return numColorList; }
            unsigned getNumNormalList() const   { return numNormalList; }
            unsigned getNumTexcoordList() const { return numTexcoordList; }
            unsigned getNumMaterials() const    { return numMaterials; }
            unsigned getNumTextures() const     { return numTextures; }
            unsigned getNumGeoStates() const    { return numGeoStates; }
            unsigned getNumGeosets() const      { return numGeoSets; }
            unsigned getNumNodes() const { return numNodes; }

            /// @}
            
            /// @name List creations
            /// @{
            
            void createLengthLists(unsigned num);
            void createTexcoordLists(unsigned num);
            void createNormalLists(unsigned num);
            void createVertexLists(unsigned num);
            void createColorLists(unsigned num);
            void createMaterials(unsigned num);
            void createTextures(unsigned num);
            void createGeoStates(unsigned num);
            void createGeoSets(unsigned num);
            void createNodes(unsigned num);

            /// @}

        private:
            PfbLengthList *lengthList;
            PfbVertexList *vertexList;
            PfbColorList *colorList;
            PfbNormalList *normalList;
            PfbTexcoordList *texcoordList;
            PfbMaterial  *materials;
            PfbTexture   *textures;
            PfbGeoState  *geostates;
            PfbGeoSet    *geosets;
            PfbNode      *nodes;

            unsigned numLengthList;
            unsigned numVertexList;
            unsigned numColorList;
            unsigned numNormalList;
            unsigned numTexcoordList;
            unsigned numMaterials;
            unsigned numTextures;
            unsigned numGeoStates;
            unsigned numGeoSets;
            unsigned numNodes;
    };
   
    struct PfbString; 
    struct PfbNodeEnd;

    /// @class PfbFile
    ///
    /// @brief PFB Loader
    class PfbFile
    {
        struct PfbHeader
        {
            uint32_t magic; // 0xdb0ace00
            uint32_t unknown1;
            uint32_t unknown2;
            uint32_t unknown3;
        };

        public:
            PfbFile(const std::string &name);
            ~PfbFile();

            std::auto_ptr<PfbTree> load();
            bool loadFailed() const;
            const char *getError() const;

        private:
            std::string   name;
            FILE    *f;
            PfbTree *tree;
            char    *error;

            PfbHeader readHeader();

            void readLengthList(PfbLengthList &list);
            void readLengthLists();

            void readVertexList(PfbVertexList &list);
            void readVertexLists();
            
            void readColorList(PfbColorList &list);
            void readColorLists();

            void readNormalList(PfbNormalList &list);
            void readNormalLists();
            
            void readTexcoordList(PfbTexcoordList &list);
            void readTexcoordLists();
            
            void readMaterial(PfbMaterial &material);
            void readMaterials();
            
            void readTexture(PfbTexture &texture);
            void readTextures();

            void readGeoState(PfbGeoState &geostate);
            void readGeoStates();
            
            void readGeoSets();

            void readChilds(PfbChilds &childs);

            void readNodeLOD(PfbNodeLOD &lod);
            void readNodeGeode(PfbNodeGeode &geode);
            void readNodeGroup(PfbNodeGroup &group);

            void readNodeSCS(PfbNodeSCS &scs);
            void readNodeDCS(PfbNodeDCS &dcs);

            void readNode(PfbNode &node);
            void readNodes();
            
            void skipBlock();
            void readNext();

            void readString(PfbString &str);
            void readNodeEnd(PfbNodeEnd &nodeEnd, long namePosition);
    };
}

extern "C" {
    const char *OpenPfb_GetVersion();
};

#endif
