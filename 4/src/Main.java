public class Main extends Thread {

    private static final int NUMBER_OF_MEN = 3;
    private static final int NUMBER_OF_WOMEN = 4;

    public static void main(String args[]){
        Bathroom bathroom = new Bathroom();
        Man menArray[] = new Man[NUMBER_OF_MEN];
        Woman womenArray[] = new Woman[NUMBER_OF_WOMEN];
        for(int i = 0; i < menArray.length; i++){
            menArray[i] = new Man(bathroom);
        }
        for(int i = 0; i < womenArray.length; i++){
            womenArray[i] = new Woman(bathroom);
        }
        for(int i = 0; i < womenArray.length; i++){
            womenArray[i].start();
        }
        for(int i = 0; i < menArray.length; i++){
            menArray[i].start();
        }
    }

}
