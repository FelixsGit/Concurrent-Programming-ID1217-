import java.util.concurrent.ThreadLocalRandom;

public class Woman extends Thread {

    Bathroom bathroom;

    public Woman(Bathroom bathroom){
        this.bathroom = bathroom;
    }

    public void run(){
        while(true) {
            int workTime = ThreadLocalRandom.current().nextInt(1, 5000 + 1);
            int bathroomTime = ThreadLocalRandom.current().nextInt(1, 2000 + 1);
            try {
                sleep(workTime);
            } catch (InterruptedException e) {
                e.printStackTrace();
            }
            try {
                bathroom.womanEnter(getId() - 10);
            } catch (InterruptedException e) {
                e.printStackTrace();
            }
            try {
                sleep(bathroomTime);
            } catch (InterruptedException e) {
                e.printStackTrace();
            }
            bathroom.womanExit(getId() - 10);
        }
    }
}
