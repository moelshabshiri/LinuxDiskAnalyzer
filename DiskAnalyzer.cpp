#include <sys/types.h>
#include <dirent.h>
#include <errno.h>
#include <vector>
#include <string>
#include <string.h>
#include <iostream>
#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <queue>
#include <stdlib.h>
#include <iostream>
#include <vector>
#include <algorithm>

using namespace std;

struct Node
{
    string path;
    string pathName; //name of the directory
    int size;
    string type;
    int level;
    vector<Node *> child;
    Node *Parent;
};

Node *newNode(string path, string pathName, int size, string type, int level, Node *Parent)
{
    Node *temp = new Node;
    temp->path = path;
    temp->pathName = pathName;
    temp->size = size;
    temp->type = type;
    temp->level = level;
    temp->Parent = Parent;
    return temp;
}

//this function prints the tree using levelordertraversal
void printTree(Node *root)
{
    if (root == NULL)
        return;
    queue<Node *> q; // Create a queue
    q.push(root);    // Enqueue root
    while (!q.empty())
    {
        int n = q.size();
        // If this node has children
        while (n > 0)
        {
            // Dequeue an item from queue and print it
            Node *p = q.front();
            Node *c = p->Parent;
            q.pop();
            cout << p->pathName << " " << p->size << " " << c->pathName << " ";

            // Enqueue all children of the dequeued item
            for (int i = 0; i < p->child.size(); i++)
                q.push(p->child[i]);
            n--;
        }
        cout << endl
             << endl; // Print new line between two levels
    }
}
//this function sorts the vector whenever a directory is added to it based on size
void sortFunc(vector<Node *> &root, int numofElements)
{
    for (int i = 0; i < (numofElements)-1; i++)
        for (int j = i + 1; j < numofElements; j++)
        {
            if ((root[i]->size) < (root[j]->size))
            {
                swap(root[i], root[j]);
            }
        }
    return;
}
//the objective of this function is to insert the directory if it is of level>2
void insert(Node *root, string name, int levelinc, int level, int size, string type, string originalPath, string secondGrandParent)
{
    int numofElements;
    if (levelinc == level - 2) //if the level of the directory-2(since we are starting at level 3 in this function) is equal to the levelinc, then the directory is pushed to the tree
    {
        (root->size += size);
        (root->child).push_back(newNode(originalPath, name, size, type, level, root));
        numofElements = (root->child).size();
        sortFunc(root->child, numofElements);
        return;
    }
    int counter = 0;
    //the objective of this loop and the code below it is to find the ancestors of the directory in the tree and then go down in the tree recursively until its parent is found .
    //when the parent is found then the directory is added using the if condition above.
    while (true)
    {

        if (root->child[counter]->path == secondGrandParent)
        {
            break;
        }
        counter++;
    }

    string OPcopy = originalPath.substr(secondGrandParent.length() + 1);
    int post = OPcopy.find('/');
    OPcopy = OPcopy.substr(0, post);
    secondGrandParent = secondGrandParent + '/' + OPcopy;

    (root->size += size);

    return insert(root->child[counter], name, levelinc + 1, level, size, type, originalPath, secondGrandParent);
}
//the objective of this function is to insert to the tree the directories if they are in levels 1 or 2
//level 0 is the root directory
void initialInsertion(string rootName, string type, Node *parent, string path, int level, struct dirent *dirp, struct stat st)
{
    int numofElements;
    if (level == 1) //if the directory is level 1, then it is added directory to the tree as the child of the root directory
    {
        (parent->child).push_back(newNode(path, dirp->d_name, st.st_size, type, level, parent));
        numofElements = (parent->child).size();

        sortFunc(parent->child, numofElements);
    }
    else if (level > 1)
    {
        int pos = (path.find(rootName) + rootName.length());
        string relativePath = path.substr(pos + 1, path.length()); // relativepath is the path of the of the of the of the current file/folder without the rootname

        int slashPos = relativePath.find("/");
        string grandParentName = relativePath.substr(0, slashPos);
        string grandParentPath = (rootName + '/' + grandParentName); //grantparentpath is the path of the children of the root= rootname/grandparentname

        int i = 0;
        while (true) //the objective of the loop is to find the child of the root, that is the ancestor of the current directory
        {
            if (parent->child[i]->path == grandParentPath) //grandParentPath
                break;
            else
                i++;
        }
        if (level == 2) //if the current directory is a level 2, then it is directly added as the child if the directory we found from the loop above
        {

            (parent->child[i]->child).push_back(newNode(path, dirp->d_name, st.st_size, type, level, parent->child[i]));
            parent->child[i]->size += st.st_size;
            numofElements = (parent->child[i]->child).size();
            sortFunc(parent->child[i]->child, numofElements);
        }
        else // if the level of the current directory is of level more than 2, then the directory gets inserted in the tree using the insert function.
        {
            //extracting the path of the second grandparent which is the children of the the grandparent
            string actPath = relativePath.substr(slashPos + 1);
            int actPathSlashPos = actPath.find("/");
            string actPathName = actPath.substr(0, actPathSlashPos);        //name of the secondgrandparent
            string secondGrandParent = grandParentPath + '/' + actPathName; //path of the second grandparent

            insert(parent->child[i], dirp->d_name, 0, level, st.st_size, "folder", path, secondGrandParent);
        }
    }
}
//objective of this function is to get the directories recursively
int getdir(string dir, Node *parent, int level, string rootName)
{
    int total = 0; //
    DIR *dp;
    struct dirent *dirp;
    struct stat st; //to get the size of the directory
    if ((dp = opendir(dir.c_str())) == NULL)
    {
        cout << "Error(" << errno << ") opening " << dir << endl;
        return errno;
    }

    while ((dirp = readdir(dp)) != NULL)
    {
        string path;
        path = dir + '/' + dirp->d_name;

        int n = path.length();
        char DirPath[n + 1];
        strcpy(DirPath, path.c_str());

        if (strcmp(dirp->d_name, ".") == 0 || strcmp(dirp->d_name, "..") == 0)
            continue;

        if (dirp->d_type == DT_DIR) //if the type is folder
        {
            stat(DirPath, &st);
            total += st.st_size; //calculate the total size of the directory recursively
            cout << dirp->d_name << " Size: " << total << " Level: " << level << endl;
            initialInsertion(rootName, "folder", parent, path, level, dirp, st);
            total += getdir(path, parent, level + 1, rootName); //since its a folder then the function will be accessed again in order to get whats inside it
        }
        else //if the type is file
        {
            stat(DirPath, &st);
            total += st.st_size;
            cout << "---" << dirp->d_name << " Size: " << st.st_size << " Level: " << level << endl;
            initialInsertion(rootName, "file", parent, path, level, dirp, st);
        }
    }

    closedir(dp); //close the directory after everything in it is inserted in the tree
    return total;
}

int main()
{
    string dir = "/home/moelshabshiri/Desktop";
    int totalSize = 0;

    Node *nullp;
    Node *root = newNode(dir, dir, totalSize, "folder", 0, nullp);

    totalSize = getdir(dir, root, 1, dir);
    root->size = totalSize;
    printTree(root);
    //  cout << endl
    //       << root->child[8]->child[0]->child[10]->pathName << endl;
    //cout << root->size;

    return 0;
}