import edu.washington.cs.diamond.Diamond;
import edu.washington.cs.diamond.ReactiveManager;
import edu.washington.cs.diamond.ReactiveManager.TxnFunction;

public class Client {
    public static void main(String[] args) throws InterruptedException {
        if (args.length < 1) {
            System.err.println("usage: java Client config_file");
            System.exit(1);
        }
        String configFile = args[0];

        Diamond.DiamondInit(configFile, 1, 0);

        final Diamond.DString str = new Diamond.DString();
        Diamond.DObject.Map(str, "javareactivetest:str");

        ReactiveManager.reactive_txn(new TxnFunction() {
            public void func(Object...args) {
                System.out.println("String value: " + str.Value());
            }
        });

        while(true) {
            Thread.sleep(1000);
        }
    }
}
