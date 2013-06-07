#include "OpenPfb.h"
#include <stack>

// Test program

#define SHELL_END    "\033[0m"
#define SHELL_RED    "\033[0;31m"
#define SHELL_GREEN  "\033[0;32m"

void testTree(openpfb::PfbTree *tree)
{
    if (!tree->haveMaterials())
        printf("BUT, no materials\n");

    std::stack<uint32_t> pfbStack;
    pfbStack.push(0);

    while (!pfbStack.empty())
    {
        uint32_t nodeId = pfbStack.top();
        if (nodeId >= tree->getNumNodes()) {
            fprintf(stderr, "Bad node ID! (%d/%d)\n", nodeId, tree->getNumNodes()); exit(1);
        }
        openpfb::PfbNode  &pfbNode      = tree->getNode(nodeId);
        pfbStack.pop();

        if (pfbNode.asGeode())
        {
            openpfb::PfbNodeGeode *geode = pfbNode.asGeode();
            for (unsigned gs = 0; gs < geode->getNumGeosets(); ++gs)
            {
                openpfb::PfbGeoSet &geoset = tree->getGeoSet(geode->getGeosets()[gs]);

                if (geoset.lengthListId == -1) {
                    fprintf(stderr, "Node %d, %s\n", nodeId, pfbNode.getName());
                    fprintf(stderr, "GeoSet: %d\n", geode->getGeosets()[gs]);
                    fprintf(stderr, "LengthListId: %d\n", geoset.lengthListId);
                    fprintf(stderr, "stripType: %d\n", geoset.stripType);
                    fprintf(stderr, "numStrip: %d\n", geoset.numStrip);
                }

                if (geoset.geostateId >= (int)tree->getNumGeoStates())
                    fprintf(stderr, "GeoState: %d/%d", geoset.geostateId, tree->getNumGeoStates());

                if (geoset.lengthListId != -1) {
                    openpfb::PfbLengthList &lengthList = tree->getLengthList(geoset.lengthListId);
                    if (lengthList.getSize() != geoset.numStrip) {
                        fprintf(stderr, "ERROR! geoset.numStrip != lengthList.getSize()\n");
                    }

                    //fprintf(stderr, "Strips: %d [%p]\n", lengthList.getSize(), lengthList.get(gs));

                    if (tree->haveVertexList()) {
                        volatile openpfb::PfbVertexList &list = tree->getVertexList(geoset.lengthListId);
                        //fprintf(stderr, "Vertex: %d [%p]\n", list.getSize(), list.get(0));
                    }
                    if (tree->haveNormalList()) {
                        volatile openpfb::PfbNormalList &list = tree->getNormalList(geoset.lengthListId);
                        //fprintf(stderr, "Normals: %d [%p]\n", list.getSize(), list.get(0));
                    }
                    if (tree->haveTexcoordList()) {
                        volatile openpfb::PfbTexcoordList &list = tree->getTexcoordList(geoset.lengthListId);
                        //fprintf(stderr, "Tex0: %d [%p]\n", list.getSize(), list.get(0));
                    }
                }
            }
        }
        else if (pfbNode.asSCS())
        {
            openpfb::PfbNodeSCS *scs   = pfbNode.asSCS();
            for (uint32_t i=scs->getChilds().getNumChildren(); i-->0;)
                pfbStack.push(scs->getChilds().getChild(i));
        }
        else if (pfbNode.asDCS())
        {
            openpfb::PfbNodeDCS *dcs   = pfbNode.asDCS();
            for (uint32_t i=dcs->getChilds().getNumChildren(); i-->0;)
                pfbStack.push(dcs->getChilds().getChild(i));
        }
        else if (pfbNode.asLOD())
        {
            openpfb::PfbNodeLOD *lod = pfbNode.asLOD();
            for (uint32_t i=0; i<lod->getChilds().getNumChildren(); i++)
                pfbStack.push(lod->getChilds().getChild(i));
        }
        else if (pfbNode.asGroup())
        {
            openpfb::PfbNodeGroup *group = pfbNode.asGroup();
            for (uint32_t i=group->getChilds().getNumChildren(); i-->0;)
                pfbStack.push(group->getChilds().getChild(i));
        }
        else
        {
            printf("WARNING: Unrecognized PfbNode.\n");
        }
    }
}

int main(int argc, char *argv[])
{
    if (argc < 2) {
        printf("usage: %s <pfbfile>\n", argv[0]);
        return 1;
    }
    char *fileName = argv[1];
    //printf("OpenPfb version: %s\n", OpenPfb_GetVersion());

    openpfb::PfbFile pfbFile(fileName);
    std::auto_ptr<openpfb::PfbTree> tree = pfbFile.load();
    if (pfbFile.loadFailed()) {
        printf(SHELL_RED "Failure: '%s' [%s]\n" SHELL_END, fileName, pfbFile.getError());
        return 1;
    }
    else {
        testTree(tree.get());
        printf(SHELL_GREEN "Success '%s'\n" SHELL_END, fileName);
        return 0;
    }
}
