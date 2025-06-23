#include "../lib/operations.h"
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>



int
fs_mkdir(file_system *fs, char *path) {
    if(!path || path[0]!='/') return -1;
    if(strlen(path)==1) return -1;
    
    char p[strlen(path)+1];strcpy(p,path);
    char *ls=strrchr(p,'/');
    
    if(ls==p) {
        char *n=path+1;
        if(!strlen(n)||strlen(n)>=NAME_MAX_LENGTH) return -1;
        
        inode *pr=&fs->inodes[fs->root_node];
        int i;for(i=0;i<DIRECT_BLOCKS_COUNT;i++) {
            if(pr->direct_blocks[i]!=-1) {
                inode *ch=&fs->inodes[pr->direct_blocks[i]];
                if(!strcmp(ch->name,n)) return -1;
            }
        }
        
        int fi=find_free_inode(fs);if(fi==-1) return -1;
        fs->inodes[fi].n_type=directory;
        strncpy(fs->inodes[fi].name,n,NAME_MAX_LENGTH);
        fs->inodes[fi].name[NAME_MAX_LENGTH-1]='\0';
        fs->inodes[fi].size=0;fs->inodes[fi].parent=fs->root_node;
        
        for(i=0;i<DIRECT_BLOCKS_COUNT;i++) fs->inodes[fi].direct_blocks[i]=-1;
        
        for(i=0;i<DIRECT_BLOCKS_COUNT;i++) {
            if(pr->direct_blocks[i]==-1) {
                pr->direct_blocks[i]=fi;return 0;
            }
        }
        fs->inodes[fi].n_type=free_block;return -1;
    }
    
    *ls='\0';char *pp=p;char *dn=ls+1;
    if(!strlen(dn)||strlen(dn)>=NAME_MAX_LENGTH) return -1;
    
    int pi=-1;
    if(!strlen(pp)) pi=fs->root_node;
    else {
        char *t=strtok(pp+1,"/");pi=fs->root_node;
        while(t) {
            int f=-1;inode *c=&fs->inodes[pi];
            if(c->n_type!=directory) return -1;
            int j;for(j=0;j<DIRECT_BLOCKS_COUNT;j++) {
                if(c->direct_blocks[j]!=-1) {
                    inode *ch=&fs->inodes[c->direct_blocks[j]];
                    if(ch->n_type==directory&&!strcmp(ch->name,t)) {
                        f=c->direct_blocks[j];break;
                    }
                }
            }
            if(f==-1) return -1;
            pi=f;t=strtok(NULL,"/");
        }
    }
    
    inode *par=&fs->inodes[pi];
    int m;for(m=0;m<DIRECT_BLOCKS_COUNT;m++) {
        if(par->direct_blocks[m]!=-1) {
            inode *ch=&fs->inodes[par->direct_blocks[m]];
            if(!strcmp(ch->name,dn)) return -1;
        }
    }
    
    int ni=find_free_inode(fs);if(ni==-1) return -1;
    fs->inodes[ni].n_type=directory;
    strncpy(fs->inodes[ni].name,dn,NAME_MAX_LENGTH);
    fs->inodes[ni].name[NAME_MAX_LENGTH-1]='\0';
    fs->inodes[ni].size=0;fs->inodes[ni].parent=pi;
    
    int k;for(k=0;k<DIRECT_BLOCKS_COUNT;k++) fs->inodes[ni].direct_blocks[k]=-1;
    
    for(k=0;k<DIRECT_BLOCKS_COUNT;k++) {
        if(par->direct_blocks[k]==-1) {
            par->direct_blocks[k]=ni;return 0;
        }
    }
    fs->inodes[ni].n_type=free_block;return -1;
}
int
fs_mkfile(file_system *fs, char *path_and_name) {
    if(!path_and_name || path_and_name[0]!='/') return -1;
    if(strlen(path_and_name)==1) return -1;
    
    char p[strlen(path_and_name)+1];strcpy(p,path_and_name);
    char *ls=strrchr(p,'/');
    
    if(ls==p) {
        char *fn=path_and_name+1;
        if(!strlen(fn)||strlen(fn)>=NAME_MAX_LENGTH) return -1;
        
        // Duplikat-Check Root
        inode *rt=&fs->inodes[fs->root_node];
        int i;for(i=0;i<DIRECT_BLOCKS_COUNT;i++) {
            if(rt->direct_blocks[i]!=-1) {
                inode *ch=&fs->inodes[rt->direct_blocks[i]];
                if(!strcmp(ch->name,fn)) return -2;
            }
        }
        
        int fi=find_free_inode(fs);if(fi==-1) return -1;
        fs->inodes[fi].n_type=reg_file;
        strncpy(fs->inodes[fi].name,fn,NAME_MAX_LENGTH);
        fs->inodes[fi].name[NAME_MAX_LENGTH-1]='\0';
        fs->inodes[fi].size=0;fs->inodes[fi].parent=fs->root_node;
        
        for(i=0;i<DIRECT_BLOCKS_COUNT;i++) fs->inodes[fi].direct_blocks[i]=-1;
        
        for(i=0;i<DIRECT_BLOCKS_COUNT;i++) {
            if(rt->direct_blocks[i]==-1) {
                rt->direct_blocks[i]=fi;return 0;
            }
        }
        fs->inodes[fi].n_type=free_block;return -1;
    }
    
    *ls='\0';char *pp=p;char *fn=ls+1;
    if(!strlen(fn)||strlen(fn)>=NAME_MAX_LENGTH) return -1;
    
    int pi=-1;
    if(!strlen(pp)) pi=fs->root_node;
    else {
        char *t=strtok(pp+1,"/");pi=fs->root_node;
        while(t) {
            int f=-1;inode *c=&fs->inodes[pi];
            if(c->n_type!=directory) return -1;
            int j;for(j=0;j<DIRECT_BLOCKS_COUNT;j++) {
                if(c->direct_blocks[j]!=-1) {
                    inode *ch=&fs->inodes[c->direct_blocks[j]];
                    if(ch->n_type==directory&&!strcmp(ch->name,t)) {
                        f=c->direct_blocks[j];break;
                    }
                }
            }
            if(f==-1) return -1;
            pi=f;t=strtok(NULL,"/");
        }
    }
    
    // Duplikat-Check Parent
    inode *par=&fs->inodes[pi];
    int m;for(m=0;m<DIRECT_BLOCKS_COUNT;m++) {
        if(par->direct_blocks[m]!=-1) {
            inode *ch=&fs->inodes[par->direct_blocks[m]];
            if(!strcmp(ch->name,fn)) return -2;
        }
    }
    
    int ni=find_free_inode(fs);if(ni==-1) return -1;
    fs->inodes[ni].n_type=reg_file;
    strncpy(fs->inodes[ni].name,fn,NAME_MAX_LENGTH);
    fs->inodes[ni].name[NAME_MAX_LENGTH-1]='\0';
    fs->inodes[ni].size=0;fs->inodes[ni].parent=pi;
    
    int k;for(k=0;k<DIRECT_BLOCKS_COUNT;k++) fs->inodes[ni].direct_blocks[k]=-1;
    
    for(k=0;k<DIRECT_BLOCKS_COUNT;k++) {
        if(par->direct_blocks[k]==-1) {
            par->direct_blocks[k]=ni;return 0;
        }
    }
    fs->inodes[ni].n_type=free_block;return -1;
}


static int copy_dir_rec(file_system *fs, int si, int di) {
    inode *sd=&fs->inodes[si];
    int i,j,k;for(i=0;i<DIRECT_BLOCKS_COUNT;i++) {
        if(sd->direct_blocks[i]!=-1) {
            int ci=sd->direct_blocks[i];inode *ch=&fs->inodes[ci];
            int ni=find_free_inode(fs);if(ni==-1) return -1;
            
            fs->inodes[ni].n_type=ch->n_type;
            strncpy(fs->inodes[ni].name,ch->name,NAME_MAX_LENGTH);
            fs->inodes[ni].name[NAME_MAX_LENGTH-1]='\0';
            fs->inodes[ni].size=0;fs->inodes[ni].parent=di;
            
            for(j=0;j<DIRECT_BLOCKS_COUNT;j++) fs->inodes[ni].direct_blocks[j]=-1;
            
            inode *dd=&fs->inodes[di];int a=0;
            for(j=0;j<DIRECT_BLOCKS_COUNT;j++) {
                if(dd->direct_blocks[j]==-1) {
                    dd->direct_blocks[j]=ni;a=1;break;
                }
            }
            if(!a) {fs->inodes[ni].n_type=free_block;return -1;}
            
            if(ch->n_type==reg_file) {
                fs->inodes[ni].size=ch->size;
                for(j=0;j<DIRECT_BLOCKS_COUNT;j++) {
                    if(ch->direct_blocks[j]!=-1) {
                        int fb=-1;
                        for(k=0;k<fs->s_block->num_blocks;k++) {
                            if(fs->free_list[k]==1) {fb=k;break;}
                        }
                        if(fb==-1) return -1;
                        memcpy(&fs->data_blocks[fb],&fs->data_blocks[ch->direct_blocks[j]],sizeof(data_block));
                        fs->free_list[fb]=0;fs->inodes[ni].direct_blocks[j]=fb;
                    }
                }
            } else if(ch->n_type==directory) {
                if(copy_dir_rec(fs,ci,ni)!=0) return -1;
            }
        }
    }
    return 0;
}

int 
fs_cp(file_system *fs, char *src_path, char *dst_path_and_name) {
    if(!src_path||!dst_path_and_name||src_path[0]!='/'||dst_path_and_name[0]!='/') return -1;
    
    int si=-1;
    if(strlen(src_path)==1) si=fs->root_node;
    else {
        char sc[strlen(src_path)+1];strcpy(sc,src_path);
        char *t=strtok(sc+1,"/");int ci=fs->root_node;
        while(t) {
            int f=-1;inode *c=&fs->inodes[ci];
            if(c->n_type!=directory) return -1;
            int i;for(i=0;i<DIRECT_BLOCKS_COUNT;i++) {
                if(c->direct_blocks[i]!=-1) {
                    inode *ch=&fs->inodes[c->direct_blocks[i]];
                    if(!strcmp(ch->name,t)) {f=c->direct_blocks[i];break;}
                }
            }
            if(f==-1) return -1;
            ci=f;t=strtok(NULL,"/");
        }
        si=ci;
    }
    if(si==-1) return -1;
    
    char dc[strlen(dst_path_and_name)+1];strcpy(dc,dst_path_and_name);
    char *ls=strrchr(dc,'/');int pi;char *nn;
    
    if(ls==dc) {pi=fs->root_node;nn=dst_path_and_name+1;}
    else {
        *ls='\0';nn=ls+1;
        if(!strlen(dc)) pi=fs->root_node;
        else {
            char *t=strtok(dc+1,"/");int ci=fs->root_node;
            while(t) {
                int f=-1;inode *c=&fs->inodes[ci];
                if(c->n_type!=directory) return -1;
                int i;for(i=0;i<DIRECT_BLOCKS_COUNT;i++) {
                    if(c->direct_blocks[i]!=-1) {
                        inode *ch=&fs->inodes[c->direct_blocks[i]];
                        if(!strcmp(ch->name,t)) {f=c->direct_blocks[i];break;}
                    }
                }
                if(f==-1) return -1;
                ci=f;t=strtok(NULL,"/");
            }
            pi=ci;
        }
    }
    
    if(pi==-1||fs->inodes[pi].n_type!=directory) return -1;
    if(!strlen(nn)||strlen(nn)>=NAME_MAX_LENGTH) return -1;
    
    inode *p=&fs->inodes[pi];int i;
    for(i=0;i<DIRECT_BLOCKS_COUNT;i++) {
        if(p->direct_blocks[i]!=-1) {
            inode *ch=&fs->inodes[p->direct_blocks[i]];
            if(!strcmp(ch->name,nn)) return -2;
        }
    }
    
    int ni=find_free_inode(fs);if(ni==-1) return -1;
    inode *s=&fs->inodes[si];
    fs->inodes[ni].n_type=s->n_type;
    strncpy(fs->inodes[ni].name,nn,NAME_MAX_LENGTH);
    fs->inodes[ni].name[NAME_MAX_LENGTH-1]='\0';
    fs->inodes[ni].size=0;fs->inodes[ni].parent=pi;
    
    for(i=0;i<DIRECT_BLOCKS_COUNT;i++) fs->inodes[ni].direct_blocks[i]=-1;
    
    int a=0;for(i=0;i<DIRECT_BLOCKS_COUNT;i++) {
        if(p->direct_blocks[i]==-1) {p->direct_blocks[i]=ni;a=1;break;}
    }
    if(!a) {fs->inodes[ni].n_type=free_block;return -1;}
    
    if(s->n_type==reg_file) {
        inode *d=&fs->inodes[ni];d->size=s->size;
        for(i=0;i<DIRECT_BLOCKS_COUNT;i++) {
            if(s->direct_blocks[i]!=-1) {
                int fb=-1;int j;
                for(j=0;j<fs->s_block->num_blocks;j++) {
                    if(fs->free_list[j]==1) {fb=j;break;}
                }
                if(fb==-1) return -1;
                memcpy(&fs->data_blocks[fb],&fs->data_blocks[s->direct_blocks[i]],sizeof(data_block));
                fs->free_list[fb]=0;d->direct_blocks[i]=fb;
            }
        }
    } else if(s->n_type==directory) {
        if(copy_dir_rec(fs,si,ni)!=0) return -1;
    }
    
    return 0;
}

char *
fs_list(file_system *fs, char *path)
{
    // Validate path
    if (path == NULL || path[0] != '/') {
        return NULL;
    }

    // Find the directory inode
    int dir_inode_idx = -1;

    if (strlen(path) == 1) {
        // Root directory
        dir_inode_idx = fs->root_node;
    } else {
        // Navigate through path
        char path_copy[strlen(path) + 1];
        strcpy(path_copy, path);

        char *token = strtok(path_copy + 1, "/"); // Skip leading '/'
        int current_inode = fs->root_node;

        while (token != NULL) {
            int found = -1;
            inode *current = &fs->inodes[current_inode];

            if (current->n_type != directory) {
                return NULL;
            }

            for (int i = 0; i < DIRECT_BLOCKS_COUNT; i++) {
                if (current->direct_blocks[i] != -1) {
                    inode *child = &fs->inodes[current->direct_blocks[i]];
                    if (child->n_type == directory && 
                        strcmp(child->name, token) == 0) {
                        found = current->direct_blocks[i];
                        break;
                    }
                }
            }

            if (found == -1) {
                return NULL;
            }

            current_inode = found;
            token = strtok(NULL, "/");
        }
        dir_inode_idx = current_inode;
    }

    // Check if it's a directory
    if (fs->inodes[dir_inode_idx].n_type != directory) {
        return NULL;
    }

    // Count entries and calculate needed size
    inode *dir = &fs->inodes[dir_inode_idx];
    int entry_count = 0;
    int total_size = 0;

    // First pass: count entries and calculate size
    for (int i = 0; i < DIRECT_BLOCKS_COUNT; i++) {
        if (dir->direct_blocks[i] != -1) {
            entry_count++;
            inode *child = &fs->inodes[dir->direct_blocks[i]];
            // "DIR " or "FIL " (4) + name + "\n" (1)
            total_size += 4 + strlen(child->name) + 1;
        }
    }

    // Allocate result string (add 1 for null terminator)
    char *result = malloc(total_size + 1);
    if (result == NULL) {
        return NULL;
    }
    result[0] = '\0';

    // Create array of entries for sorting
    typedef struct {
        int inode_idx;
        char *name;
        int is_dir;
    } Entry;

    if (entry_count == 0) {
        return result; // Empty directory
    }

    Entry entries[entry_count];
    int entry_idx = 0;

    // Collect all entries
    for (int i = 0; i < DIRECT_BLOCKS_COUNT; i++) {
        if (dir->direct_blocks[i] != -1) {
            inode *child = &fs->inodes[dir->direct_blocks[i]];
            entries[entry_idx].inode_idx = dir->direct_blocks[i];
            entries[entry_idx].name = child->name;
            entries[entry_idx].is_dir = (child->n_type == directory);
            entry_idx++;
        }
    }

    // Sort by inode index (simple bubble sort)
    for (int i = 0; i < entry_count - 1; i++) {
        for (int j = 0; j < entry_count - i - 1; j++) {
            if (entries[j].inode_idx > entries[j + 1].inode_idx) {
                Entry temp = entries[j];
                entries[j] = entries[j + 1];
                entries[j + 1] = temp;
            }
        }
    }

    // Build result string
    for (int i = 0; i < entry_count; i++) {
        if (entries[i].is_dir) {
            strcat(result, "DIR ");
        } else {
            strcat(result, "FIL ");
        }
        strcat(result, entries[i].name);
        strcat(result, "\n");
    }

    return result;
}

int
fs_writef(file_system *fs, char *filename, char *text)
{
	return -1;
}

uint8_t *
fs_readf(file_system *fs, char *filename, int *file_size)
{
	return NULL;
}


int
fs_rm(file_system *fs, char *path)
{
	return -1;
}

int
fs_import(file_system *fs, char *int_path, char *ext_path)
{
	return -1;
}

int
fs_export(file_system *fs, char *int_path, char *ext_path)
{
	return -1;
}
