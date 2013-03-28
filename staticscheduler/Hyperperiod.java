package staticscheduler;


import java.util.Comparator;
import java.util.Iterator;
import java.util.List;
import java.util.ArrayList;
import java.util.PriorityQueue;
import java.util.TreeSet;


public class Hyperperiod
{
	public Hyperperiod()
	{
		this.taskSet = new ArrayList<Task>();
		this.frameList = new ArrayList<Frame>();
		clear();
	}
	
	public int getFrameSize() { return this.periodMin; }
	public int getFrameCount() 
	{
		if( this.periodMin == 0 ) 
			return 0;
		
		return this.periodLCM / this.periodMin; 
	}	
	
	public int getLength() { return this.periodLCM; }
	
	public boolean addTask( Task newTask )
	{
		addPeriod( newTask.getPeriod() );
		System.err.println( "Adding task to hyperperiod: " + newTask.toString() );
		return taskSet.add( newTask );
	}
	
	public void printTasks()
	{
		Iterator<Task> tasks = this.taskSet.iterator();
		
		while( tasks.hasNext() )
		{
			Task currTask = tasks.next();
			System.out.println( "Show|" + currTask.toString() );
		}
	}
	
	public void printSchedule() throws ScheduleException
	{
		if( ! this.initialized )
			initializeFrames();
		
		if( ! this.scheduled )
			schedule();
		
		Iterator<Frame> frames = frameList.iterator();
		
		while( frames.hasNext() )
		{
			Frame currFrame = frames.next();
			System.out.println( "Show|" + currFrame.toStringWithSchedule() );
		}
	}
	
	public void initializeFrames ()
	{
		int frameSize = getFrameSize();
		int frameCount = getFrameCount();
		
		if( this.frameList.size() > 0 )
			this.frameList.clear();
		
		Frame.ResetID();
		Job.ResetID();
		
		for( int i = 0; i < frameCount; ++i )
		{
			Frame newFrame = new Frame( (i * frameSize), frameSize );
			System.err.println( "Adding frame to hyperperiod: " + newFrame.toString() );
			this.frameList.add( i, newFrame );
		}
		
		initialized = true;
	}
	
	public void schedule() throws ScheduleException
	{
		Job currJob;
		Iterator<Task> tasks = taskSet.iterator();
		
		Comparator<Job> compare = new JobWCETDeadlineReleaseComparator();
		PriorityQueue<Job> allJobs = new PriorityQueue<Job>( taskSet.size(), compare );
		
		while( tasks.hasNext() )
		{
			Task currTask = tasks.next();
			System.err.println( "Scheduling jobs for task: " + currTask.toString() );
			
			List<Job> taskJobs = currTask.getJobs( this.getLength() );
			
			Iterator<Job> jobs = taskJobs.iterator();
			
			while( jobs.hasNext() )
			{
				currJob = jobs.next();
				if(! allJobs.add( currJob ) )
				{
					throw new ScheduleException("Failed to add job to set");
				}
			}
		}
		
		currJob = allJobs.poll();
		while( currJob != null )
		{			
			System.out.println( "Show|" + currJob.toString() );

			if( ! scheduleJobInFrame( currJob ) )
			{
				System.out.println("Fatal error | Could not schedule job (" + currJob.getIdentifier() + ").");
				return;
			}
			
			currJob = allJobs.poll();
		}
	}
	
	private boolean scheduleJobInFrame( Job jobOfInterest ) throws ScheduleException
	{
		TreeSet<Frame> eligibleFrames = getEligibleFramesForJob ( jobOfInterest );
		
		if ( eligibleFrames.isEmpty() )
		{
			System.out.println("Error | No eligible frames for Job: " + jobOfInterest.toString() );
			return false;
		}

		Iterator<Frame> frames = eligibleFrames.iterator();
		
		while( frames.hasNext() )
		{
			Frame currFrame = frames.next();
			
			if( currFrame.addJob( jobOfInterest ) )
			{
				System.err.println( "Success | Job " + jobOfInterest.getIdentifier() + " scheduled in Frame " + currFrame.getIdentifier() + "." );
				System.err.println( currFrame.toStringWithSchedule() );
				return true;
			}
		}
		
		String errorStr = "Job " + jobOfInterest.getIdentifier() + " could not be scheduled in eligible frames:" + System.getProperty( "line.separator" );
		frames = eligibleFrames.iterator();
		while( frames.hasNext() )
		{
			Frame currFrame = frames.next();
			errorStr += currFrame.toStringWithSchedule() + System.getProperty( "line.separator" );
		}
		throw new ScheduleException( errorStr );
	}
	
	private TreeSet<Frame> getEligibleFramesForJob ( Job insideJob )
	{	
		// earliest frame it could finish in
		int firstFrameIndex = ( ( insideJob.getReleaseTime() + insideJob.getWCET() ) / this.getFrameSize() );
		
		// latest frame it could start in 
		int lastFrameIndex = ( ( insideJob.getDeadline() - insideJob.getWCET() ) / this.getFrameSize() );
		
		System.err.println( "EligibleFrame|FirstIndex: " + firstFrameIndex + "|LastIndex: " + lastFrameIndex );
		
		// return frames in order of decreasing remaining capacity, favoring later frame start time where remaining capacity is equal
		// this is critical... I could not schedule the work loads if I was returning the frames 
		// in increasing start time order, without considering remaining capacity
		Comparator<Frame> compare = new FrameRemainingCapacityStartTimeComparator();
		TreeSet<Frame> eligibleFrames = new TreeSet<Frame>( compare );
	
		// it's possible that this loop will not be entered (unschedulable job)
		for( int frameIndex = firstFrameIndex; frameIndex <= lastFrameIndex; ++frameIndex )
		{
			eligibleFrames.add( this.frameList.get( frameIndex ) );
		}
		
		return eligibleFrames;
	}
	
	public void clear ()
	{
		initialized = false;
		scheduled = false;
		periodMin = 0;
		periodLCM = 0;
		taskSet.clear();
		frameList.clear();
	}
	
	public static int lcm( int a, int b ) 
	{
		// Note: large periods could cause overflow
		// http://stackoverflow.com/questions/4201860/how-to-find-gcf-lcm-on-a-set-of-numbers
		return a * ( b / gcd( a, b ) );
	}
	
	public static int gcd ( int a, int b )
	{
		// Euclid's Algorithm
		// http://stackoverflow.com/questions/4201860/how-to-find-gcf-lcm-on-a-set-of-numbers
		while ( b > 0 )
		{
			int temp = b;
			b = a % b;
			a = temp;
		}
		
		return a;
	}
	
	// calculates the minimum period and the hyperperiod length as each task period is added
	private void addPeriod( int taskPeriod )
	{
		if( ( this.periodMin == 0 ) || ( taskPeriod < this.periodMin ) )
			this.periodMin = taskPeriod;
		
		if( this.periodLCM ==  0 )
			this.periodLCM = taskPeriod;
		else
			this.periodLCM = lcm( this.periodLCM, taskPeriod );
	}
	
	private int periodMin;
	private int periodLCM;
	private boolean initialized;
	private boolean scheduled;
	
	private ArrayList<Task> taskSet;
	private ArrayList<Frame> frameList;
}