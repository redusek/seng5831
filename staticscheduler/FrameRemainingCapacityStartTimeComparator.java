package staticscheduler;

import java.util.Comparator;

// sort frames in order of decreasing remaining capacity, decreasing start time
public class FrameRemainingCapacityStartTimeComparator implements Comparator<Frame>
{
	public int compare( Frame x, Frame y)
	{
		// sort tasks in order of decreasing remaining capacity
		if( x.remainingCapacity() > y.remainingCapacity() )
		{
			return -1;
		}
		else if( x.remainingCapacity() < y.remainingCapacity() )
		{
			return 1;
		}
		else
		{
			// decreasing start time
			if( x.getStartTime() > y.getStartTime() )
			{
				return -1;
			}
			else if( x.getStartTime() < y.getStartTime() )
			{
				return 1;
			}
		}
		
		return 0;
	}
}