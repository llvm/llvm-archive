public class objinst {
    private static class Toggle {
        boolean state = true;
        public Toggle(boolean start_state) {
            this.state = start_state;
        }
        public boolean value() {
            return(this.state);
        }
        public Toggle activate() {
            this.state = !this.state;
            return(this);
        }
    }

    private static class NthToggle extends Toggle {
        int count_max = 0;
        int counter = 0;

        public NthToggle(boolean start_state, int max_counter) {
            super(start_state);
            this.count_max = max_counter;
            this.counter = 0;
        }
        public Toggle activate() {
            this.counter += 1;
            if (this.counter >= this.count_max) {
                this.state = !this.state;
                this.counter = 0;
            }
            return(this);
        }
    }

    public static void main(String args[]) {
        int n = 1500000;
        Toggle toggle1 = new Toggle(true);
        for (int i=0; i<5; i++) {
            Test.println(toggle1.activate().value());
        }
        for (int i=0; i<n; i++) {
            Toggle toggle = new Toggle(true);
        }

        NthToggle ntoggle1 = new NthToggle(true, 3);
        for (int i=0; i<8; i++) {
            Test.println(ntoggle1.activate().value());
        }
        for (int i=0; i<n; i++) {
            NthToggle toggle = new NthToggle(true, 3);
        }
    }
}
