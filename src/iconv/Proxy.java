package iconv;

public class Proxy {
	public static native void init();
	public static native boolean test1();
	
	static {
		System.loadLibrary("iconv");
	}
}
