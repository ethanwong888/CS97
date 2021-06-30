import os
import sys
import zlib

#step 1 - discover the .git directory inside the larger Git repository
def discover_git_dir():
    #determine the current path 
    current = os.getcwd()
    file_name = ".git"

    while True:
        #list the files in the current directory
        all_files = os.listdir(current)
        #determine the parent directory
        parent = os.path.dirname(current)

        #iterate through all_files, find one called ".git" -> return the path
        if file_name in all_files:
            return os.path.realpath(os.path.join(current,".git"));
        #if ".git" was not found, iterate to the parent directory
        else:
            if current == parent:
                #if this is the top directory, throw an error
                print("Not inside a Git repository") >> sys.stderr
                sys.exit(1)
            else:
                current = parent



#step 2 - create list of local branch names by looking in .git/refs/heads
def build_local_branches(branch_dict=dict()):
    #join /refs/heads to the path of the git directory
    branch_list = []
    heads = os.path.join(os.path.join(discover_git_dir(), "refs"), "heads")

    #iterate through the tree to all files in this directory 
    for root, _dir, file in os.walk(heads):
        for f in file:
            p = os.path.join(root, f)
            #remove the head portion and add only the name of the file to the list of branches
            if p.startswith(heads):
                p = p[len(heads)+1:]
            branch_list.append(p)

    #iterate through the list of branches and hash them in the dictionary 
    for b in branch_list:
        #we need to replace the hashes ending in \n with nothing
        new_hash = open(os.path.join(heads, b), 'r').read().replace('\n', '')
        if hash not in branch_dict:
            branch_dict[new_hash] = []
        branch_dict[new_hash].append(b)



#step 3 - building the graph of all the commits
def build_commit_graph(branch_dict, commit_dict=[]):
    #look through the .git/objects directory and find the commit objects
    objects = os.path.realpath(os.path.join(discover_git_dir(), "objects"))

    for b in branch_dict:
        #add the branch head to the stack
        stack = []
        stack.append(b)
        #start the depth-first-search
        while(len(stack)!= 0):
            c = stack.pop()
            if c not in commit_dict:
                commit_dict[c] = CommitNode(c)
            s = ""
            s +=(c[0:2])
            s += "/"
            s += c[2:]

            #find all parents of the commit and update the dictionary accordingly
            decompress = zlib.decompress(open(os.path.realpath(os.path.join(objects,s)), 'rb').read())
            d = decompress.decode()
            for i in d.split("\n"):
                if i.startswith("parent "):
                    parent = i[7:]
                    stack.append(parent)
                    if parent not in commit_dict:
                      commit_dict[parent] = CommitNode(parent)
                    commit_dict[c].parents.add(parent)
                    commit_dict[parent].children.add(c)



#step 4 - topologically sort the commit graph
def sort_commit_graph(commit_dict, root_commits, sorted_commits=[]):
    seen = set()

    #loop to topologically sort
    while len(root_commits)!=0:
        node = commit_dict[root_commits[-1]]
        e = False
        for i in node.children:
            #find the first unvisited child and add it to root_commits
            if i not in seen:
                root_commits.append(i)
                e = True
                break
        #when there aren't any unvisited children, delete the first root commit on the stack and add it to the graph
        if e == False:
            root_commits.pop()
            sorted_commits.append(node.commit_hash)
            seen.add(node.commit_hash)
            


def topo_order_commits():
    #combine all the previous steps/functions together and use them
    #i.e, build the commit graph and local branches, then sort the commit graph and print out with proper formatting
    commit_dict = dict()
    branch_dict = dict()
    root_commits = []
    sorted_commits = []
    build_local_branches(branch_dict)
    build_commit_graph(branch_dict, commit_dict)

    #find the root commits
    for i in commit_dict:
        if not commit_dict[i].parents:
            root_commits.append(i)

    sort_commit_graph(commit_dict, sorted(root_commits), sorted_commits)
    #print commit hash
    for i, c in enumerate(sorted_commits):
        #if an empty line was just printed, print a "Sticky start"
        #"Sticky start" is a line with an "=" preceded be the hashes for the children
        if i > 0 and sorted_commits[i-1] not in commit_dict[c].children:
            print('=', end = '')
            print(*list(commit_dict[c].children))
        #if a commit corresponds to a branch head/heads, branch names should be listed after the commit in order
        if c in branch_dict:
            print(c, end = ' ')
            print(*sorted(branch_dict[c]), sep = ' ')
        else:
            print(c)

        #insert a "Sticky end" if the next commit isn't the parent of the current commit
        #"Sticky end" has the commit hashes for the parents of the current commit, with an =
        #just print an = if there aren't any parents
        if i+1 < len(sorted_commits) and sorted_commits[i+1] not in commit_dict[c].parents:
            print(*list(commit_dict[c].parents), sep = ' ', end = '')
            print('=\n')
    return 0           
                


class CommitNode:
    def __init__(self, commit_hash):
        """                                                                                                                         
        :type commit_hash: str                                                                                                      
        """
        self.commit_hash = commit_hash
        self.parents = set()
        self.children = set()



if __name__ == '__main__':
    topo_order_commits()


#As I finished each step for this assignment, I used strace to check if I used any other commands that I shouldn't have, particularly 'git' commands
#If there were any 'git' commands found, I would try to rework my implementation to get rid of them
#I always used the command 'strace -i' so that the strace command could be interrupted if I needed to do so. This prevents it from running infinitely as I can stop the command whenever I want using CTRL-C.
#I would typically use the command 'strace -e EXPR' to trace the use of the EXPR within my program
#I also tried to use 'strace -f' to trace child processes as I developed incrementally
#I tried using regex with my strace commands just for fun, but I found that it was much easier without the regex
#I would send the output of strace into a file every time I used it so I could look through it and see what commands were being used
#The main command I used was 'strace -i python3 topo_order_commits.py 2 > test_log'