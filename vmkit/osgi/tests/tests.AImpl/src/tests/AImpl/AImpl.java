package tests.AImpl;

import java.util.ArrayList;

import tests.A.A;
import tests.B.BEvent;

public class AImpl
	implements A, BEvent
{
	ArrayList<Integer> items;
	
	public AImpl()
	{
		items = new ArrayList<Integer>();
	}
	
	public void a()
	{
		allocObj(100);
		
		System.out.println(A.class.getName() + ".a() called.");
		
		sleep(1);
	}
	
	void allocObj(int count)
	{
		try {p(count);} catch (Exception e) {}
	}
	
	void p(int count)
	{
		for (int i=0; i<count; ++i)
			items.add(new Integer((int)Math.random()));
	}

	public int b(long x, long y)
	{
		return 0;
	}

	public long[] c(int n)
	{
		return null;
	}

	public void handler()
	{
//		allocObj(10000);
//		System.out.println(BEvent.class.getName() + ".handler() called.");

		ArrayList<Double> T = new ArrayList<Double>();
		for (int i=0; i < 49999; ++i)
			T.add(new Double(Math.random()));
		
		sleep(5);
	}
	
	void sleep(long sec)
	{
		try {
			Thread.sleep(sec * 1000);
		} catch (Exception e) {e.printStackTrace();}
	}
}
