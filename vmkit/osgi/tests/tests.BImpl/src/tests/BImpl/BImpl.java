package tests.BImpl;

import tests.A.A;
import tests.B.BEvent;

public class BImpl
	implements Runnable
{
	A a;
	BEvent b;
	boolean termThread;
	Thread th;
	
	public BImpl()
	{
		th = new Thread(this, this.getClass().getName());
	}
	
	public void start()
	{
		termThread = false;
		th.start();
	}
	
	public void stop()
	{
		termThread = true;
		
		try {
			th.join();
		} catch (Exception e) {e.printStackTrace();}
	}

	public void setA(A a)
	{
		this.a = a;
		
		if (a == null)
			System.out.println(this.getClass().getPackage().getName() + " no more references " + A.class.getName());
		else
			System.out.println(this.getClass().getPackage().getName() + " references " + A.class.getName());
	}
	
	public void setBEvent(BEvent b)
	{
		this.b = b;
		
		if (b == null)
			System.out.println(this.getClass().getPackage().getName() + " no more references " + BEvent.class.getName());
		else
			System.out.println(this.getClass().getPackage().getName() + " references " + BEvent.class.getName());
	}
	
	void dummy
	(
		long d00, long d01, long d02, long d03, long d04, long d05, long d06, long d07, long d08, long d09,
		long d10, long d11, long d12, long d13, long d14, long d15, long d16, long d17, long d18, long d19,
		long d20, long d21, long d22, long d23, long d24, long d25, long d26, long d27, long d28, long d29,
		long d30, long d31, long d32, long d33, long d34, long d35, long d36, long d37, long d38, long d39,
		long d40, long d41, long d42, long d43, long d44, long d45, long d46, long d47, long d48, long d49,
		long d50, long d51, long d52, long d53, long d54, long d55, long d56, long d57, long d58, long d59,
		long d60, long d61, long d62, long d63, long d64, long d65, long d66, long d67, long d68, long d69,
		long d70, long d71, long d72, long d73, long d74, long d75, long d76, long d77, long d78, long d79,
		long d80, long d81, long d82, long d83, long d84, long d85, long d86, long d87, long d88, long d89,
		long d90, long d91, long d92, long d93, long d94, long d95, long d96, long d97, long d98, long d99)
	{
		try {
			Thread.sleep(1000);
		} catch (Exception e) {}
	}

	public void run()
	{
		long v00 = 0, v01 = 0, v02 = 0, v03 = 0, v04 = 0, v05 = 0, v06 = 0, v07 = 0, v08 = 0, v09 = 0;
		long v10 = 0, v11 = 0, v12 = 0, v13 = 0, v14 = 0, v15 = 0, v16 = 0, v17 = 0, v18 = 0, v19 = 0;
		long v20 = 0, v21 = 0, v22 = 0, v23 = 0, v24 = 0, v25 = 0, v26 = 0, v27 = 0, v28 = 0, v29 = 0;
		long v30 = 0, v31 = 0, v32 = 0, v33 = 0, v34 = 0, v35 = 0, v36 = 0, v37 = 0, v38 = 0, v39 = 0;
		long v40 = 0, v41 = 0, v42 = 0, v43 = 0, v44 = 0, v45 = 0, v46 = 0, v47 = 0, v48 = 0, v49 = 0;
		long v50 = 0, v51 = 0, v52 = 0, v53 = 0, v54 = 0, v55 = 0, v56 = 0, v57 = 0, v58 = 0, v59 = 0;
		long v60 = 0, v61 = 0, v62 = 0, v63 = 0, v64 = 0, v65 = 0, v66 = 0, v67 = 0, v68 = 0, v69 = 0;
		long v70 = 0, v71 = 0, v72 = 0, v73 = 0, v74 = 0, v75 = 0, v76 = 0, v77 = 0, v78 = 0, v79 = 0;
		long v80 = 0, v81 = 0, v82 = 0, v83 = 0, v84 = 0, v85 = 0, v86 = 0, v87 = 0, v88 = 0, v89 = 0;
		long v90 = 0, v91 = 0, v92 = 0, v93 = 0, v94 = 0, v95 = 0, v96 = 0, v97 = 0, v98 = 0, v99 = 0;

		dummy(v00, v01, v02, v03, v04, v05, v06, v07, v08, v09, v10,
			v11, v12, v13, v14, v15, v16, v17, v18, v19, v20,
			v21, v22, v23, v24, v25, v26, v27, v28, v29, v30,
			v31, v32, v33, v34, v35, v36, v37, v38, v39, v40,
			v41, v42, v43, v44, v45, v46, v47, v48, v49, v50,
			v51, v52, v53, v54, v55, v56, v57, v58, v59, v60,
			v61, v62, v63, v64, v65, v66, v67, v68, v69, v70,
			v71, v72, v73, v74, v75, v76, v77, v78, v79, v80,
			v81, v82, v83, v84, v85, v86, v87, v88, v89, v90,
			v91, v92, v93, v94, v95, v96, v97, v98, v99);

		System.out.println(this.getClass().getPackage().getName() + " running...");
/*
		Object[] o = new Object[100];
		for (int i=0; i < 100; ++i)
			o[i] = new Object();

		for (int i=0; i < 100; ++i) {
			synchronized(o[i]) {
				o[i].notify();
			}
		}
*/		
		do_it();
		
		System.out.println(this.getClass().getPackage().getName() + " stopped.");
	}
	
	void do_it()
	{
//		long v01 = 0, v02 = 0, v03 = 0, v04 = 0, v05 = 0, v06 = 0, v07 = 0, v08 = 0, v09 = 0, v10 = 0, v11 = 0, v12 = 0, v13 = 0, v14 = 0, v15 = 0, v16 = 0, v17 = 0, v18 = 0, v19 = 0, v20 = 0, v21 = 0, v22 = 0, v23 = 0, v24 = 0, v25 = 0, v26 = 0, v27 = 0, v28 = 0, v29 = 0, v30 = 0, v31 = 0, v32 = 0, v33 = 0, v34 = 0, v35 = 0, v36 = 0, v37 = 0, v38 = 0, v39 = 0, v40 = 0, v41 = 0, v42 = 0, v43 = 0, v44 = 0, v45 = 0, v46 = 0, v47 = 0, v48 = 0, v49 = 0, v50 = 0, v51 = 0;
//		dummy(v01, v02, v03, v04, v05, v06, v07, v08, v09, v10, v11, v12, v13, v14, v15, v16, v17, v18, v19, v20, v21, v22, v23, v24, v25, v26, v27, v28, v29, v30, v31, v32, v33, v34, v35, v36, v37, v38, v39, v40, v41, v42, v43, v44, v45, v46, v47, v48, v49, v50, v51);
				
		while (!termThread) {
			if (a == null)
				sleep(1);
			else
				a.a();

			if (b == null)
				sleep(1);
			else
				b.handler();
		}
	}
	
	void sleep(long sec)
	{
		try {
			Thread.sleep(sec * 1000);
		} catch (Exception e) {e.printStackTrace();}
	}
}
