package staticscheduler;

import java.util.Comparator;

public class TaskPriorityComparator implements Comparator<Task>
{
	public int compare( Task x, Task y)
	{
		if( x.getPriority() < y.getPriority() )
			return -1;
		
		if( x.getPriority() == y.getPriority() )
		{
			if( x.getWCET() < y.getWCET() )
				return -1;
			
			if( x.getWCET() == y.getWCET() )
				return 0;
			
			return 1;
		}
		
		return 1;
	}
}
