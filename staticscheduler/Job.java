package staticscheduler;

public class Job
{
	// this class assumes that the offset time for the task is built into the release time for the job
	// so, the release time for the job is the release time for the task, plus the offset for the task
	public Job ( String parentTask, int releaseT, int wcet, int deadline, int priority ) 		
	{
		// should probably be throwing an exception if release + wcet > deadline
		// but, we only allow input from user in Task, and that does throw exception if task 
		// has nonsense attributes that can't be scheduled (like release > deadline)
		// so, we're kind of safe for now
		this.releaseT = releaseT;
		this.wcet = wcet;
		this.deadline = deadline;
		this.priority = priority;
		
		this.identifier = Job.GetNextID();
		this.parentTask = parentTask;
	}
		
	public int getReleaseTime ()
	{
		return this.releaseT;
	}
	
	public int getDeadline()
	{
		return this.deadline;
	}
	
	public int getPriority()
	{
		return this.priority;
	}
	
	public int getWCET()
	{
		return this.wcet;
	}
	
	public String getIdentifier()
	{
		return this.identifier;
	}
	
	public int timeBeforeRelease ( int t )
	{
		return ( this.releaseT <= t ) ? 0 : ( this.releaseT - t );
	}
	
	public boolean canScheduleAt ( int t )
	{
		return ( ( t >= this.releaseT ) && ( meetDeadlineStartingAt( t ) ) );
	}
	
	public boolean meetDeadlineStartingAt ( int t )
	{
		return ( t + this.wcet <= this.deadline );
	}
	
	// returns finish time based on start time
	public int getFinishTimeFromStartTime ( int t )
	{
		return t + this.wcet;
	}
	
	public String toString ()
	{
		return "Job|ID:" + this.identifier + "|R:" + this.releaseT + "|E:" + this.wcet + "|D:" + this.deadline + "|Pr:" + this.priority + "|Task:" + this.parentTask;
	}
	
	public String getParentTask () 
	{
		return parentTask;
	}
	
	public static void ResetID()
	{
		Job.count = 0;
	}
	
	public static void SetPrefix ( String jobPrefix )
	{
		Job.jobPrefix = jobPrefix;
	}
	
	private static String GetNextID()
	{
		return Job.jobPrefix + Integer.toString(++Job.count);
	}
		
	private int releaseT;
	private int wcet;
	private int deadline;
	private int priority;
	private String identifier;
	private String parentTask;
	private static int count = 0;
	private static String jobPrefix = "J";
}