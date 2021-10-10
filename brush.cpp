#include "brush.h"
#include "hlbsp.h"
#include <stdio.h>
#include <time.h>

float calc_distance(vec3 point, Plane plane)
{
    return dot(point, plane.normal) - plane.distance;
}

int32 Environment::new_vertexNode()
{
    vertexTree.push_back({});
    return vertexTree.size()-1;
}

int32 Environment::new_vertex()
{
    vertices.push_back({});
    return vertices.size()-1;
}

int32 Environment::new_plane()
{
    planes.push_back({});
    return planes.size()-1;
}

int32 Environment::new_link()
{
    links.push_back({});
    return links.size()-1;
}

int32 Environment::new_face()
{
    faces.push_back({});
    return faces.size()-1;
}

int32 Environment::new_subface()
{
    subfaces.push_back({});
    return subfaces.size()-1;
}

int32 Environment::new_brush()
{
    brushes.push_back({});
    return brushes.size()-1;
}

void Environment::clear()
{
    vertexTree.clear();
    vertexTree.push_back({{nill, nill, nill, nill}});
    vertices.clear();
    planes.clear();
    links.clear();
    faces.clear();
    subfaces.clear();
    brushes.clear();
}

int32 Environment::init_box(vec3 min, vec3 max)
{
    int32 p = planes.size();
    int32 l = links.size();
    int32 f = faces.size();
    int32 s = subfaces.size();
    int32 b = brushes.size();

    int32 v[8];

    v[0] = add_vertex({min.x, min.y, min.z});
    v[1] = add_vertex({min.x, min.y, max.z});
    v[2] = add_vertex({min.x, max.y, min.z});
    v[3] = add_vertex({min.x, max.y, max.z});
    v[4] = add_vertex({max.x, min.y, min.z});
    v[5] = add_vertex({max.x, min.y, max.z});
    v[6] = add_vertex({max.x, max.y, min.z});
    v[7] = add_vertex({max.x, max.y, max.z});

    planes.push_back({{+1, 0, 0}, +min.x});
    planes.push_back({{-1, 0, 0}, -max.x});
    planes.push_back({{0, +1, 0}, +min.y});
    planes.push_back({{0, -1, 0}, -max.y});
    planes.push_back({{0, 0, +1}, +min.z});
    planes.push_back({{0, 0, -1}, -max.z});
    
    //face 0
    links.push_back({v[0], l+1});
    links.push_back({v[2], l+2});
    links.push_back({v[3], l+3});
    links.push_back({v[1], l+0});

    //face 1
    links.push_back({v[4], l+5});
    links.push_back({v[5], l+6});
    links.push_back({v[7], l+7});
    links.push_back({v[6], l+4});

    //face 2
    links.push_back({v[0], l+ 9});
    links.push_back({v[1], l+10});
    links.push_back({v[5], l+11});
    links.push_back({v[4], l+ 8});

    //face 3
    links.push_back({v[2], l+13});
    links.push_back({v[6], l+14});
    links.push_back({v[7], l+15});
    links.push_back({v[3], l+12});

    //face 4
    links.push_back({v[0], l+17});
    links.push_back({v[4], l+18});
    links.push_back({v[6], l+19});
    links.push_back({v[2], l+16});

    //face 5
    links.push_back({v[1], l+21});
    links.push_back({v[3], l+22});
    links.push_back({v[7], l+23});
    links.push_back({v[5], l+20});

    //subfaces
    links.push_back({s+0, l+24});
    links.push_back({s+1, l+25});
    links.push_back({s+2, l+26});
    links.push_back({s+3, l+27});
    links.push_back({s+4, l+28});
    links.push_back({s+5, l+29});

    //faces
    links.push_back({f+0, l+31});
    links.push_back({f+1, l+32});
    links.push_back({f+2, l+33});
    links.push_back({f+3, l+34});
    links.push_back({f+4, l+35});
    links.push_back({f+5, l+30});
    
    faces.push_back({l+0,  l+24, p+0});
    faces.push_back({l+4,  l+25, p+1});
    faces.push_back({l+8,  l+26, p+2});
    faces.push_back({l+12, l+27, p+3});
    faces.push_back({l+16, l+28, p+4});
    faces.push_back({l+20, l+29, p+5});

    int32 m = SUBFACE_SOLID | SUBFACE_EMPTY;

    subfaces.push_back({l+ 0, {nill, nill}, m});
    subfaces.push_back({l+ 4, {nill, nill}, m});
    subfaces.push_back({l+ 8, {nill, nill}, m});
    subfaces.push_back({l+12, {nill, nill}, m});
    subfaces.push_back({l+16, {nill, nill}, m});
    subfaces.push_back({l+20, {nill, nill}, m});

    brushes.push_back({l+30, nill, {nill, nill}, 0});
    
    return b;
}

int32 Environment::add_vertex(vec3 vertex)
{
    //logarithmic time - fast
    int32 node = 0;
    for(int32 i = 0; i < 3; i++)
    for(int32 j = 0; j < 16; j++)
    {
        int32 data0 = ((int32*)&vertex)[i];
        int32 data1 = (data0 >> (2*j)) & 0x3;

        if(i == 2 && j == 15)
        {
            int32 value = vertexTree[node].down[data1];
            if(value == nill)
            {
                value = new_vertex();
                vertices[value] = vertex;
                vertexTree[node].down[data1] = value;
            }
            return value;
        }

        int32 next = vertexTree[node].down[data1];
        if(next == nill)
        {
            next = new_vertexNode();
            vertexTree[node].down[data1] = next;
            vertexTree[next] = {nill, nill, nill, nill};
        }
        node = next;
    }
    
    /*
    //linear time - slow
    for(int32 i = 0; i < vertices.size(); i++)
        if(vertices[i] == vertex)
            return i;

    int32 n = new_vertex();
    vertices[n] = vertex;
    return n;
    */
}

int32 Environment::add_vertex(int32 vertex_a, float dist_a, int32 vertex_b, float dist_b)
{
    if(vertex_a > vertex_b)
    {
        int32 vc = vertex_a;
        float dc = dist_a;
        vertex_a = vertex_b;
        dist_a = dist_b;
        vertex_b = vc;
        dist_b = dc;
    }

    vec3 p = (dist_a * vertices[vertex_b] - dist_b * vertices[vertex_a])/(dist_a - dist_b);

    return add_vertex(p);
}

void Environment::add_link(int32 *link, int32 data)
{
    int32 n = new_link();

    if(*link != nill)
    {
        links[n].lNext = links[*link].lNext;
        links[*link].lNext = n;
    }
    else links[n].lNext = n;

    *link = n;
    links[n].data = data;
}

bool Environment::check_face(int32 lastLink, int32 plane, bool *side)
{
    int32 link = lastLink;
    
    do
    {
        int32 next = links[link].lNext;

        vec3 a = vertices[links[link].data];
        vec3 b = vertices[links[next].data];

        float dist_a = calc_distance(a, planes[plane]);
        float dist_b = calc_distance(b, planes[plane]);

        *side = dist_a < 0;
        if((dist_a < 0) != (dist_b < 0)) return true;

        link = next;
    } while(link != lastLink);

    return false;
}

void Environment::cut_face(int32 lastLink, int32 newLinks[2], int32 newVertices[2], int32 plane)
{
    int32 link = lastLink;

    do
    {
        int32 next = links[link].lNext;

        vec3 a = vertices[links[link].data];
        vec3 b = vertices[links[next].data];

        float dist_a = calc_distance(a, planes[plane]);
        float dist_b = calc_distance(b, planes[plane]);

        add_link(&newLinks[dist_a < 0], links[link].data);

        if((dist_a < 0) != (dist_b < 0))
        {
            int32 c = add_vertex(links[link].data, dist_a, links[next].data, dist_b);
            add_link(&newLinks[dist_a < 0], c);
            add_link(&newLinks[dist_b < 0], c);
            newVertices[dist_a < 0] = c;
        }

        link = next;
    } while(link != lastLink);
}

void Environment::cut_subface(int32 *link[2], int32 subface, int32 plane)
{
    bool side;
    bool cut = check_face(subfaces[subface].lEdges, plane, &side);

    if(cut)
    {
        if(subfaces[subface].children[0] == nill)
        {
            int32 newLinks[2] = {nill, nill};
            int32 newVertices[2];

            cut_face(subfaces[subface].lEdges, newLinks, newVertices, plane);

            for(int32 i = 0; i < 2; i++)
            {
                int32 s = new_subface();
                subfaces[subface].children[i] = s;
                subfaces[s] = {newLinks[i], {nill, nill}, subfaces[subface].contents};
                add_link(link[i], s);
            }
        }
        else
            for(int32 i = 0; i < 2; i++)
                cut_subface(link, subfaces[subface].children[i], plane);
    }
    else
        add_link(link[side], subface);
}

void Environment::cut_brush(int32 brush, int32 plane)
{
    brushes[brush].plane = plane;
    int32 newFaces[2] = {nill, nill};

    for(int32 i = 0; i < 2; i++)
    {
        int32 child = new_brush();
        brushes[brush].children[i] = child;
        brushes[child] = {nill, nill, {nill, nill}, 0};
    }

    struct Pair { int32 data[2]; };
    vector<Pair> pairs;

    int32 lastLink = brushes[brush].lFaces;
    int32 link = lastLink;
    bool side;

    do
    {
        int32 ind = links[link].data;
        bool sign = ind < 0;

        int32 face = sign ? ~ind : ind;

        if(check_face(faces[face].lEdges, plane, &side))
        {
            int32 newLinks[2] = {nill, nill};
            int32 newVertices[2];

            cut_face(faces[face].lEdges, newLinks, newVertices, plane);

            int32 subLinks[2] = {nill, nill};

            int32 fs[2];
            for(int32 i = 0; i < 2; i++)
            {
                int32 f = new_face();
                fs[i] = f;
                faces[f] = {newLinks[i], nill, faces[face].plane};
                add_link(&newFaces[i], sign ? ~f : f);
            }

            if(newVertices[0] != newVertices[1])
                pairs.push_back({newVertices[1 ^ sign], newVertices[0 ^ sign]});

            int32 lastLink = faces[face].lSubfaces;
            int32 link = lastLink;

            if(link != nill)
            do
            {
                int32 *ptr[2] = {&subLinks[0], &subLinks[1]};
                cut_subface(ptr, links[link].data, plane);
                link = links[link].lNext;
            } while(link != lastLink);

            for(int32 i = 0; i < 2; i++)
                faces[fs[i]].lSubfaces = subLinks[i];

        }
        else
            add_link(&newFaces[side], ind);

        link = links[link].lNext;
    } while(link != lastLink);

    lastLink = nill;

    for(int32 i = 0; i < pairs.size(); i++)
    {
        for(int32 j = i+1; j < pairs.size(); j++)
        {
            if(pairs[i].data[1] == pairs[j].data[0])
            {
                Pair tmp = pairs[j];
                pairs[j] = pairs[i+1];
                pairs[i+1] = tmp;
            }
        }

        add_link(&lastLink, pairs[i].data[0]);
    }

    if(lastLink != nill)
    {
        int32 f = new_face();
        int32 s = new_subface();
        int32 k = nill;
        add_link(&k, s);
        faces[f] = {lastLink, k, plane};
        subfaces[s] = {lastLink, {nill, nill}, 0};

        add_link(&newFaces[0], f);
        add_link(&newFaces[1], ~f);
    }
    else brushes[brushes[brush].children[side^1]].contents = CONTENTS_DEGENERATE;

    for(int32 i = 0; i < 2; i++)
        brushes[brushes[brush].children[i]].lFaces = newFaces[i];
}

void BspExport::proc_bsp(int32 brush, int32 clipnode)
{
    env.cut_brush(brush, clipnodes[clipnode].plane);

    for(int32 i = 0; i < 2; i++)
    {
        int32 child = env.brushes[brush].children[i];

        if(env.brushes[child].contents == CONTENTS_DEGENERATE) continue;
        
        int32 next = clipnodes[clipnode].next[i];
        if(next >= 0)
            proc_bsp(child, next);
        else
            env.brushes[child].contents = next;
    }
}

void BspExport::visibility_rec(int32 subface, int32 contents)
{
    if(env.subfaces[subface].children[0] == nill)
        env.subfaces[subface].contents |= contents;
    else
        for(int32 i = 0; i < 2; i++)
            visibility_rec(env.subfaces[subface].children[i], contents);
}

void BspExport::visibility(int32 brush)
{
    int32 contents = env.brushes[brush].contents;
    if(contents == CONTENTS_DEGENERATE) return;

    if(contents == 0)
    {
        for(int32 i = 0; i < 2; i++)
            visibility(env.brushes[brush].children[i]);
        return;
    }

    int32 m = 0;
    if(contents == CONTENTS_EMPTY) m = SUBFACE_EMPTY;
    else m = SUBFACE_SOLID;

    int32 lastF = env.brushes[brush].lFaces;
    int32 f = lastF;

    do
    {
        int32 ind = env.links[f].data;
        bool sign = ind < 0;
        int32 face = sign ? ~ind : ind;

        int32 lastS = env.faces[face].lSubfaces;
        int32 s = lastS;

        if(s != nill)
        do
        {
            int32 subface = env.links[s].data;
            visibility_rec(subface, m);
            s = env.links[s].lNext;
        } while(s != lastS);

        f = env.links[f].lNext;
    } while(f != lastF);
}

bool BspExport::read_bsp(const char *filename)
{   
    printf("Processing map: %s\n", filename);
    clock_t time0 = clock();

    FILE *fp = fopen(filename, "rb");
    if(!fp) return false;

    fseek(fp, 0, SEEK_END);
    bsp_size = ftell(fp);
    fseek(fp, 0, SEEK_SET);
    bsp_data = new char[bsp_size];
    int k = fread(bsp_data, bsp_size, 1, fp);
    fclose(fp);

    header =    (HEADER*)bsp_data;
    planes =    (PLANE*)&bsp_data[header->lump[LUMP_PLANES].pos];
    models =    (MODEL*)&bsp_data[header->lump[LUMP_MODELS].pos];
    clipnodes = (CLIPNODE*)&bsp_data[header->lump[LUMP_CLIPNODES].pos];

    num_planes = header->lump[LUMP_PLANES].size / sizeof(PLANE);
    num_models = header->lump[LUMP_MODELS].size / sizeof(MODEL);
    num_clipnodes = header->lump[LUMP_CLIPNODES].size / sizeof(CLIPNODE);

    if(header->version != 30) return false;

    env.clear();

    for(int i = 0; i < num_planes; i++)
        env.planes.push_back({planes[i].normal, planes[i].dist});

    vec3 thick = {0, 0, 0};
    vec3 min = models[0].min - thick;
    vec3 max = models[0].max + thick;

    env.init_box(min, max);
    env.init_box(min, max);
    env.init_box(min, max);
    env.init_box(min, max);

    for(int i = 0; i < 4; i++)
    {
        proc_bsp(i, models[0].hull[i]);
        visibility(i);
    }

    clock_t time1 = clock();
    float time_diff = (float)(time1 - time0) / CLOCKS_PER_SEC;

    printf("Time: %fs\n", time_diff);

    return true;
}
