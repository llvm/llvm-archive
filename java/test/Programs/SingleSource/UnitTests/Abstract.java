abstract class AbstractAbstract {
    abstract int abstractMethod();

    int concreteMethod() { return abstractMethod(); }
}

class AbstractConcrete extends AbstractAbstract {
    int abstractMethod() { return 5; }
}

public class Abstract
{
    public static void main(String[] args) {
        Test.print_int_ln(new AbstractConcrete().concreteMethod());
    }
}
