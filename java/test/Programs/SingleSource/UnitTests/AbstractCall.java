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
        AbstractCallAbstract a = new AbstractCallConcrete();
        Test.print_int_ln(a.concreteMethod());
    }
}
