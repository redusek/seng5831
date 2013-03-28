package staticscheduler;

@SuppressWarnings("unused")

public final class FrameJob
{
	public FrameJob( int t, int duration )
	{
		this.idle = true;
		this.startT = t;
		this.duration = duration;
		this.taskJob = null;
	}
	
	public FrameJob( int t, Job taskJob )
	{
		this.idle = false;
		this.startT = t;
		this.taskJob = taskJob;
		this.duration = taskJob.getWCET();
	}
	
	public String getIdentifier()
	{
		if( idle ) 
			return "IDLE";
		
		return this.taskJob.getIdentifier();
	}
	
	public Job getTaskJob()
	{
		return this.taskJob;
	}
	
	public boolean isIdle()
	{
		return this.idle;
	}
	
	public int getStartTime()
	{
		return this.startT;
	}
	
	public int getDuration()
	{
		return this.duration;
	}

	public int getFinishTime()
	{
		return this.startT + this.duration;
	}
	
	public void updateStartTime( int t )
	{
		this.startT += t;
	}
	
	private int startT;
	private final int duration;
	private final boolean idle;
	private final Job taskJob;
}