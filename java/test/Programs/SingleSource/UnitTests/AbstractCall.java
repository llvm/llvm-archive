abstract class AbstractCallAbstract {
    abstract int abstractMethod();

    int concreteMethod() { return abstractMethod(); }
}

class AbstractCallConcrete extends AbstractCallAbstract {
    int abstractMethod() { return 5; }
}

public class AbstractCall
{
    public static void main(String[] args) {
        Test.print_int_ln(new AbstractCallConcrete().concreteMethod());
    }
}
