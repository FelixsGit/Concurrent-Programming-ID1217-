import java.util.concurrent.locks.Condition;
import java.util.concurrent.locks.Lock;
import java.util.concurrent.locks.ReentrantLock;

public class Bathroom {

    int menInBathroom = 0;
    int menInQueue = 0;
    int womenInBathroom = 0;
    int womenInQueue = 0;
    private Lock lock = new ReentrantLock();
    private Condition women = lock.newCondition();
    private Condition men = lock.newCondition();
    long startTime = System.currentTimeMillis();


    public void manEnter(long id) throws InterruptedException {
        lock.lock();
        if(womenInBathroom >  0 || womenInQueue > 0){
            menInQueue ++;
            System.out.println("After "+ (System.currentTimeMillis() - startTime)+ " milliseconds ---" + " man with id "+ id +" enters queue");
            men.await();
        }
        menInBathroom ++;
        if(menInQueue > 0){
            menInQueue --;
            men.signal();
        }
        System.out.println("After "+ (System.currentTimeMillis() - startTime)+ " milliseconds ---" + " man with id " + id + " enter bathroom");
        lock.unlock();
    }

    public void manExit(long id){
        lock.lock();
        menInBathroom --;
        System.out.println("After "+ (System.currentTimeMillis() - startTime)+ " milliseconds ---" + " man with id " + id + " leave bathroom");
        if(menInBathroom == 0 && womenInQueue > 0){
            System.out.println("\n---Womans turn---\n");
            womenInQueue --;
            women.signal();
        }else if(menInBathroom == 0 && womenInQueue == 0){
            System.out.println("\n---Bathroom Empty, no one in queue---\n");
        }
        lock.unlock();
    }

    public void womanEnter(long id) throws InterruptedException {
        lock.lock();
        if(menInBathroom >  0 || menInQueue > 0){
            womenInQueue ++;
            System.out.println("After "+ (System.currentTimeMillis() - startTime)+ " milliseconds ---" + " woman with id " + id + " enter queue");
            women.await();
        }
        womenInBathroom ++;
        if(womenInQueue > 0) {
            womenInQueue--;
            women.signal();
        }
        System.out.println("After "+ (System.currentTimeMillis() - startTime)+ " milliseconds ---" + " woman with id " + id  + " enter bathroom");
        lock.unlock();
    }

    public void womanExit(long id){
        lock.lock();
        womenInBathroom --;
        System.out.println("After "+ (System.currentTimeMillis() - startTime)+ " milliseconds ---" + " woman with id " + id + " leave bathroom time: ");
        if(womenInBathroom == 0 && menInQueue > 0) {
            System.out.println("\n---Mens turn---\n");
            menInQueue--;
            men.signal();
        }else if(womenInBathroom == 0 && menInQueue == 0){
            System.out.println("\n---Bathroom Empty, no one in queue---\n");
        }
        lock.unlock();
    }

}
