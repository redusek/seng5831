package staticscheduler;

import java.util.Comparator;

public class JobWCETDeadlineReleaseTimeComparator implements Comparator<Job>
{
	public int compare( Job x, Job y)
	{
		// sort tasks in order of descreasing WCET
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
			else
			{
				if( x.getReleaseTime() < y.getReleaseTime() )
				{
					return -1;
				}
				else if( x.getReleaseTime() > y.getReleaseTime() )
				{
					return 1;
				}
			}
		}
		
		return 0;
	}
}