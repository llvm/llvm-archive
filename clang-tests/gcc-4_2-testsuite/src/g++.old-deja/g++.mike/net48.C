// { dg-do assemble  }

const char *a="a\0315";

class A
{
public:
    A()
    {
        const char *b="a\0315";
    }
};

const char *c="a\0315";
