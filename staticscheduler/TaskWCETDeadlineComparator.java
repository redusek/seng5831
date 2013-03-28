package staticscheduler;

import java.util.Comparator;

public class TaskWCETDeadlineComparator implements Comparator<Task>
{
	public int compare( Task x, Task y)
	{
		// sort tasks in order of decreasing WCET
		if( x.getWCET() > y.getWCET() )
		{
			return -1;
		}
		else if( x.getWCET() < y.getWCET() )
		{
			return 1;
		} 
		else
		{
			if( x.getDeadline() < y.getDeadline() )
			{
				return -1;
			}
			else if( x.getDeadline() > y.getDeadline() )
			{
				return 1;
			}
		}
		
		return 0;
	}
}