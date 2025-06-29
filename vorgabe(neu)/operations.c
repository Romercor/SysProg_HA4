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
                        fs->free_list[fb]=0;fs->s_block->free_blocks--;fs->inodes[ni].direct_blocks[j]=fb;
                    }
                }
            } else if(ch->n_type==directory) {
                if(copy_dir_rec(fs,ci,ni)!=0) return -1;
            }
        }
    }
    return 0;
}

int fs_cp(file_system *fs, char *src_path, char *dst_path_and_name) {
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
                fs->free_list[fb]=0;fs->s_block->free_blocks--;d->direct_blocks[i]=fb;
            }
        }
    } else if(s->n_type==directory) {
        if(copy_dir_rec(fs,si,ni)!=0) return -1;
    }
    
    return 0;
}

char *
fs_list(file_system *fs, char *path) {
    if(!path||path[0]!='/') return NULL;
    
    int di=-1;
    if(strlen(path)==1) di=fs->root_node;
    else {
        char pc[strlen(path)+1];strcpy(pc,path);
        char *t=strtok(pc+1,"/");int ci=fs->root_node;
        while(t) {
            int f=-1;inode *c=&fs->inodes[ci];
            if(c->n_type!=directory) return NULL;
            int i;for(i=0;i<DIRECT_BLOCKS_COUNT;i++) {
                if(c->direct_blocks[i]!=-1) {
                    inode *ch=&fs->inodes[c->direct_blocks[i]];
                    if(ch->n_type==directory&&!strcmp(ch->name,t)) {
                        f=c->direct_blocks[i];break;
                    }
                }
            }
            if(f==-1) return NULL;
            ci=f;t=strtok(NULL,"/");
        }
        di=ci;
    }
    
    if(fs->inodes[di].n_type!=directory) return NULL;
    
    inode *d=&fs->inodes[di];int ec=0,ts=0;
    int i;for(i=0;i<DIRECT_BLOCKS_COUNT;i++) {
        if(d->direct_blocks[i]!=-1) {
            ec++;inode *ch=&fs->inodes[d->direct_blocks[i]];
            ts+=4+strlen(ch->name)+1;
        }
    }
    
    char *r=malloc(ts+1);if(!r) return NULL;r[0]='\0';
    
    typedef struct{int ii;char *n;int id;} E;
    if(!ec) return r;
    
    E e[ec];int ei=0;
    for(i=0;i<DIRECT_BLOCKS_COUNT;i++) {
        if(d->direct_blocks[i]!=-1) {
            inode *ch=&fs->inodes[d->direct_blocks[i]];
            e[ei].ii=d->direct_blocks[i];e[ei].n=ch->name;
            e[ei].id=(ch->n_type==directory);ei++;
        }
    }
    
    int j;for(i=0;i<ec-1;i++) {
        for(j=0;j<ec-i-1;j++) {
            if(e[j].ii>e[j+1].ii) {
                E tmp=e[j];e[j]=e[j+1];e[j+1]=tmp;
            }
        }
    }
    
    for(i=0;i<ec;i++) {
        if(e[i].id) strcat(r,"DIR ");
        else strcat(r,"FIL ");
        strcat(r,e[i].n);strcat(r,"\n");
    }
    
    return r;
}

int 
fs_writef(file_system *fs, char *filename, char *text) {
    if(!filename||!text||filename[0]!='/') return -1;
    
    int fi=-1;
    if(strlen(filename)==1) return -1;
    
    char pc[strlen(filename)+1];strcpy(pc,filename);
    char *t=strtok(pc+1,"/");int ci=fs->root_node;
    
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
    
    if(fs->inodes[ci].n_type!=reg_file) return -1;
    fi=ci;
    
    inode *file=&fs->inodes[fi];
    int tl=strlen(text),bw=0,cbi=-1,oib=0;
    
    int i;for(i=0;i<DIRECT_BLOCKS_COUNT;i++) {
        if(file->direct_blocks[i]!=-1) {
            cbi=i;int bn=file->direct_blocks[i];
            oib=fs->data_blocks[bn].size;
        }
    }
    
    while(bw<tl) {
        int bn=-1;
        
        if(cbi==-1||oib>=BLOCK_SIZE) {
            cbi++;
            if(cbi>=DIRECT_BLOCKS_COUNT) {
                if(!bw) return -2;
                break;
            }
            
            int fb=-1;
            for(i=0;i<fs->s_block->num_blocks;i++) {
                if(fs->free_list[i]==1) {fb=i;break;}
            }
            
            if(fb==-1) {
                if(!bw) return -2;
                break;
            }
            
            fs->free_list[fb]=0;fs->s_block->free_blocks--;
            file->direct_blocks[cbi]=fb;fs->data_blocks[fb].size=0;
            bn=fb;oib=0;
        } else {
            bn=file->direct_blocks[cbi];
        }
        
        int sib=BLOCK_SIZE-oib;
        int btw=MIN(sib,tl-bw);
        
        memcpy(&fs->data_blocks[bn].block[oib],&text[bw],btw);
        
        fs->data_blocks[bn].size=oib+btw;bw+=btw;oib+=btw;
    }
    
    file->size+=bw;
    return bw;
}


uint8_t *
fs_readf(file_system *fs, char *filename, int *file_size) {
    if(file_size) *file_size=0;
    if(!filename||filename[0]!='/') return NULL;
    
    int fi=-1;
    if(strlen(filename)==1) return NULL;
    
    char pc[strlen(filename)+1];strcpy(pc,filename);
    char *t=strtok(pc+1,"/");int ci=fs->root_node;
    
    while(t) {
        int f=-1;inode *c=&fs->inodes[ci];
        if(c->n_type!=directory) return NULL;
        int i;for(i=0;i<DIRECT_BLOCKS_COUNT;i++) {
            if(c->direct_blocks[i]!=-1) {
                inode *ch=&fs->inodes[c->direct_blocks[i]];
                if(!strcmp(ch->name,t)) {f=c->direct_blocks[i];break;}
            }
        }
        if(f==-1) return NULL;
        ci=f;t=strtok(NULL,"/");
    }
    
    if(fs->inodes[ci].n_type!=reg_file) return NULL;
    fi=ci;
    
    inode *file=&fs->inodes[fi];
    if(!file->size) return NULL;
    
    uint8_t *buf=malloc(file->size);if(!buf) return NULL;
    memset(buf,0,file->size);
    
    int br=0,i;for(i=0;i<DIRECT_BLOCKS_COUNT&&br<file->size;i++) {
        if(file->direct_blocks[i]!=-1) {
            int bn=file->direct_blocks[i];
            if(bn<0||bn>=fs->s_block->num_blocks) continue;
            
            data_block *b=&fs->data_blocks[bn];
            int bs=b->size;if(bs>BLOCK_SIZE) bs=BLOCK_SIZE;
            int btr=bs;if(br+btr>file->size) btr=file->size-br;
            
            if(btr>0) {
                memcpy(buf+br,b->block,btr);br+=btr;
            }
        }
    }
    
    if(file_size) *file_size=file->size;
    return buf;
}


static void del_rec(file_system *fs, int ii) {
    if(ii<0||ii>=fs->s_block->num_blocks) return;
    
    inode *n=&fs->inodes[ii];
    if(n->n_type==free_block) return;
    
    if(n->n_type==reg_file) {
        int i;for(i=0;i<DIRECT_BLOCKS_COUNT;i++) {
            if(n->direct_blocks[i]!=-1) {
                int bn=n->direct_blocks[i];
                if(bn>=0&&bn<fs->s_block->num_blocks) {
                    if(fs->free_list[bn]==0) {
                        fs->free_list[bn]=1;
                        if(fs->s_block->free_blocks<fs->s_block->num_blocks) {
                            fs->s_block->free_blocks++;
                        }
                    }
                }
            }
        }
    } else if(n->n_type==directory) {
        n->n_type=free_block;
        int i;for(i=0;i<DIRECT_BLOCKS_COUNT;i++) {
            if(n->direct_blocks[i]!=-1) {
                int ci=n->direct_blocks[i];
                if(ci>=0&&ci<fs->s_block->num_blocks&&ci!=ii) {
                    del_rec(fs,ci);
                }
            }
        }
    } else {
        n->n_type=free_block;
    }
    
    n->n_type=free_block;n->size=0;
    memset(n->name,0,NAME_MAX_LENGTH);n->parent=-1;
    int i;for(i=0;i<DIRECT_BLOCKS_COUNT;i++) n->direct_blocks[i]=-1;
}

int 
fs_rm(file_system *fs, char *path) {
    if(!path||path[0]!='/') return -1;
    if(strlen(path)==1) return -1;
    
    char pc[strlen(path)+1];strcpy(pc,path);
    char *ls=strrchr(pc,'/');char *in;int pi;
    
    if(ls==pc) {
        pi=fs->root_node;in=path+1;
    } else {
        *ls='\0';in=ls+1;
        char *t=strtok(pc+1,"/");int ci=fs->root_node;
        while(t) {
            int f=-1;inode *c=&fs->inodes[ci];
            if(c->n_type!=directory) return -1;
            int i;for(i=0;i<DIRECT_BLOCKS_COUNT;i++) {
                if(c->direct_blocks[i]!=-1) {
                    inode *ch=&fs->inodes[c->direct_blocks[i]];
                    if(ch->n_type==directory&&!strcmp(ch->name,t)) {
                        f=c->direct_blocks[i];break;
                    }
                }
            }
            if(f==-1) return -1;
            ci=f;t=strtok(NULL,"/");
        }
        pi=ci;
    }
    
    inode *p=&fs->inodes[pi];int iii=-1,ibs=-1;
    int i;for(i=0;i<DIRECT_BLOCKS_COUNT;i++) {
        if(p->direct_blocks[i]!=-1) {
            inode *ch=&fs->inodes[p->direct_blocks[i]];
            if(!strcmp(ch->name,in)) {
                iii=p->direct_blocks[i];ibs=i;break;
            }
        }
    }
    
    if(iii==-1) return -1;
    
    del_rec(fs,iii);
    p->direct_blocks[ibs]=-1;
    
    return 0;
}

int
fs_import(file_system *fs, char *int_path, char *ext_path)
{
    if(!int_path||!ext_path||int_path[0]!='/') return -1;
    FILE *ef=fopen(ext_path,"rb");if(!ef) return -1;
    int fii=-1;if(strlen(int_path)==1) {fclose(ef);return -1;}
    char pc[strlen(int_path)+1];strcpy(pc,int_path);
    char *t=strtok(pc+1,"/");int ci=fs->root_node;
    while(t) {
        int f=-1;inode *c=&fs->inodes[ci];
        if(c->n_type!=directory) {fclose(ef);return -1;}
        char *nt=strtok(NULL,"/");
        int i;for(i=0;i<DIRECT_BLOCKS_COUNT;i++) {
            if(c->direct_blocks[i]!=-1) {
                inode *ch=&fs->inodes[c->direct_blocks[i]];
                if(!strcmp(ch->name,t)) {
                    if(!nt&&ch->n_type==reg_file) fii=c->direct_blocks[i];
                    else if(nt&&ch->n_type==directory) f=c->direct_blocks[i];
                    break;
                }
            }
        }
        if(!nt) break;
        if(f==-1) {fclose(ef);return -1;}
        ci=f;t=nt;
    }
    if(fii==-1) {fclose(ef);return -1;}
    inode *fi=&fs->inodes[fii];int cbi=-1,oib=0;
    int i;for(i=0;i<DIRECT_BLOCKS_COUNT;i++) {
        if(fi->direct_blocks[i]!=-1) {
            cbi=i;int bn=fi->direct_blocks[i];oib=fs->data_blocks[bn].size;
        }
    }
    int bw=0;unsigned char buf[BLOCK_SIZE];
    while(!feof(ef)&&!ferror(ef)) {
        int sic=0;if(cbi>=0&&oib<BLOCK_SIZE) sic=BLOCK_SIZE-oib;
        int btr=sic>0?sic:BLOCK_SIZE;
        size_t br=fread(buf,1,btr,ef);if(br==0) break;
        if(sic==0) {
            cbi++;if(cbi>=DIRECT_BLOCKS_COUNT) break;
            int fb=-1;
            for(i=0;i<fs->s_block->num_blocks;i++) {
                if(fs->free_list[i]==1) {fb=i;break;}
            }
            if(fb==-1) {fclose(ef);return -1;}
            fs->free_list[fb]=0;if(fs->s_block->free_blocks>0) fs->s_block->free_blocks--;
            fi->direct_blocks[cbi]=fb;fs->data_blocks[fb].size=0;oib=0;
        }
        int bn=fi->direct_blocks[cbi];
        memcpy(&fs->data_blocks[bn].block[oib],buf,br);
        fs->data_blocks[bn].size=oib+br;bw+=br;oib+=br;
    }
    int he=ferror(ef);fclose(ef);
    if(fi->size>UINT16_MAX-bw) fi->size=UINT16_MAX;
    else fi->size+=bw;
    return he?-1:0;
}
int
fs_export(file_system *fs, char *int_path, char *ext_path)
{
    if(!int_path||!ext_path||int_path[0]!='/') return -1;
    int pl=strlen(int_path);
    while(pl>1&&int_path[pl-1]=='/') pl--;
    char np[pl+1];strncpy(np,int_path,pl);np[pl]='\0';
    int fii=-1;if(pl==1) return -1;
    char pc[pl+1];strcpy(pc,np);
    char *t=strtok(pc+1,"/");int ci=fs->root_node;
    while(t) {
        if(strlen(t)==0) {t=strtok(NULL,"/");continue;}
        int f=-1;inode *c=&fs->inodes[ci];
        if(c->n_type!=directory) return -1;
        char *nt=strtok(NULL,"/");
        int i;for(i=0;i<DIRECT_BLOCKS_COUNT;i++) {
            if(c->direct_blocks[i]!=-1) {
                if(c->direct_blocks[i]<0||c->direct_blocks[i]>=fs->s_block->num_blocks) continue;
                inode *ch=&fs->inodes[c->direct_blocks[i]];
                if(!strcmp(ch->name,t)) {
                    if(!nt&&ch->n_type==reg_file) fii=c->direct_blocks[i];
                    else if(nt&&ch->n_type==directory) f=c->direct_blocks[i];
                    break;
                }
            }
        }
        if(!nt) break;
        if(f==-1) return -1;
        ci=f;t=nt;
    }
    if(fii==-1) return -1;
    inode *fi=&fs->inodes[fii];if(fi->n_type!=reg_file) return -1;
    FILE *ef=fopen(ext_path,"wb");if(!ef) return -1;
    int as=0;
    int i;for(i=0;i<DIRECT_BLOCKS_COUNT;i++) {
        if(fi->direct_blocks[i]!=-1) {
            int bn=fi->direct_blocks[i];
            if(bn>=0&&bn<fs->s_block->num_blocks) {
                if(fs->free_list[bn]==0) as+=fs->data_blocks[bn].size;
            }
        }
    }
    int ste=MIN(fi->size,as);int bw=0;
    for(i=0;i<DIRECT_BLOCKS_COUNT&&bw<ste;i++) {
        if(fi->direct_blocks[i]!=-1) {
            int bn=fi->direct_blocks[i];
            if(bn<0||bn>=fs->s_block->num_blocks) continue;
            if(fs->free_list[bn]!=0) continue;
            data_block *b=&fs->data_blocks[bn];
            int btw=b->size;if(btw>BLOCK_SIZE) btw=BLOCK_SIZE;
            if(bw+btw>ste) btw=ste-bw;
            if(btw>0) {
                size_t w=fwrite(b->block,1,btw,ef);
                if(w!=btw) {fclose(ef);remove(ext_path);return -1;}
                bw+=w;
            }
        }
    }
    if(ferror(ef)) {fclose(ef);remove(ext_path);return -1;}
    if(fclose(ef)!=0) {remove(ext_path);return -1;}
    return 0;
}
