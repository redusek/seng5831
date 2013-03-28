package staticscheduler;

import java.util.Comparator;

//sort jobs in order of increasing deadline, increasing release time, and decreasing WCET
public class JobReleaseDeadlineWCETComparator implements Comparator<Job>
{
	public int compare( Job x, Job y)
	{
		// sort jobs in order of increasing release time
		if( x.getReleaseTime() < y.getReleaseTime() )
		{
			return -1;
		}
		else if( x.getReleaseTime() > y.getReleaseTime() )
		{
			return 1;
		}
		else
		{
			// if jobs have the same release time, they are sorted in order of increasing deadline
			if( x.getDeadline() < y.getDeadline() )
			{
				return -1;
			}
			else if( x.getDeadline() < y.getDeadline() )
			{
				return 1;
			}
			else 
			{
				// if jobs have the same deadline and release time, then they are sorted in order of decreasing WCET
				if( x.getWCET() > y.getWCET() )
				{
					return -1;
				}
				else if( x.getWCET() < y.getWCET() )
				{
					return 1;
				} 
			}
			
		}
		
		return 0;
	}
}