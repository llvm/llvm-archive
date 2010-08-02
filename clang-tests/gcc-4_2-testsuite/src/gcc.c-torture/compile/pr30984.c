int fs_exec(int ino)
{
 void *p = ({l: &&l; }); 
 void *src = 0;
 if (ino)
   src = (void*)0xe000;
 goto *src;
}
