package staticscheduler;

import java.util.List;
import java.util.ArrayList;

// not currently thread-safe, static class variables need mutex protection to make it thread-safe
public class Task
{
	public Task(int period, int wcet, int offset, int deadline, int priority) throws TaskException
	{
		if( period < 1 )
			throw new InvalidTaskPeriod();
		
		if( wcet < 1 )
			throw new InvalidTaskWCET();
		
		if( offset < 0 )
			throw new InvalidTaskOffset();
		
		if( deadline < wcet + offset )
			throw new InvalidTaskDeadline();
		
		if( priority < 1 )
			throw new InvalidTaskPriority();
		
		this.period = period;
		this.wcet = wcet;
		this.offset = offset;
		this.deadline = deadline;
		this.priority = priority;
		
		this.identifier = Task.GetNextID();
	}
	
	public int getPeriod ()
	{
		return period;
	}
	
	public int getWCET ()
	{
		return this.wcet;
	}
	
	public int getOffset ()
	{
		return this.offset;
	}
	
	public int getDeadline ()
	{
		return this.deadline;
	}
	
	public int getPriority ()
	{
		return this.priority;
	}

	public String getIdentifier () 
	{
		return this.identifier;
	}
	
	public List<Job> getJobs ( int hyperPeriodLength )
	{
		int numJobs = hyperPeriodLength / this.period;
		
		List<Job> jobList = new ArrayList<Job>();
		
		// we want all of the job names to indicate what task they are from
		Job.SetPrefix( this.identifier + "J" );
		Job.ResetID();  // the job numbers for each task start with 1 
		
		for( int i = 0; i < numJobs; ++i )
		{
			jobList.add( new Job( this.identifier, jobRelease( i ) , this.wcet, jobDeadline( i ), this.priority ) );
		}
		
		return jobList;
	}
	
	public String toString ()
	{
		return "Task|ID:" + this.identifier + "|P:" + this.period + "|O:" + this.offset + "|E:" + this.wcet + "|D:" + this.deadline + "|Pr:" + this.priority;
	}
	
	public static void ResetID ()
	{
		Task.count = 0;
	}
	
	public static void SetPrefix ( String prefix )
	{
		Task.taskPrefix = prefix;
	}
	
	private int jobDeadline( int jobNumber )
	{
		return ( ( jobNumber * this.period ) + this.deadline );
	}
	
	// job release time is based on task release time + offset, so a job need not concern itself with offset
	private int jobRelease( int jobNumber ) 
	{
		return ( ( jobNumber * this.period ) + this.offset );
	}
	
	private static String GetNextID ()
	{
		return taskPrefix + Integer.toString( ++count ); 
	}
	
	private final int period;
	private final int wcet;
	private final int offset;
	private final int deadline;
	private final int priority;
	private final String identifier;
	
	private static int count = 0;
	private static String taskPrefix = "A";
}
