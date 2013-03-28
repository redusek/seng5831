package staticscheduler;

import java.util.Iterator;
import java.util.LinkedList;

public class Frame
{	
	public Frame ( int startT, int duration )
	{
		// makes sure jobs are inserted in deadline order
		// Comparator<Job> compareJobs = new JobDeadlineReleaseWCETComparator();
		this.jobs = new LinkedList<FrameJob>();
		
		this.duration = duration;
		this.startT = startT;
		this.identifier = Frame.GetNextID();
	}
	
	public boolean addJob(Job newJob) throws ScheduleException
	{
		System.err.println( "Attempting to add job " + newJob.getIdentifier() + " to frame " + this.identifier );
		
		if( remainingCapacity() >= newJob.getWCET() )
		{		
			// start at the end of the list and attempt to schedule incoming job as late as possible
			// we can't schedule the incoming job any later than maxT in this frame
			int maxT = ( this.startT + this.duration ) - newJob.getWCET();
			
			// if release time is > maxT, the job cannot be scheduled in this frame
			if( newJob.getReleaseTime() > maxT )
			{
				System.err.println( "Release time for job " + newJob.getIdentifier() + " too late for frame " + this.identifier );
				return false;
			}

			int t; // time... is it on our side? 

			// special case of the empty list
			if( this.jobs.size() == 0 )
			{
				this.addFrameJob( this.startT, 0, newJob );
			}
			else
			{
				// special case for fitting on the end of the list
				t = this.jobs.getLast().getFinishTime();
				if( t <= maxT )
				{
					this.addFrameJob( t, this.jobs.size(), newJob);
				}
				else
				{
					// work our way backward through the list
					FrameJob currFrameJob = null; 
					int jobIndex = -1;
					for( jobIndex = ( jobs.size() - 1 ) ; jobIndex >= 0; --jobIndex )
					{
						currFrameJob = jobs.get( jobIndex );
						// check to see if we can fit before the current job
						t = currFrameJob.getStartTime();  
						if( t <= maxT ) 
						{
							if ( newJob.meetDeadlineStartingAt( t ) ) 
							{
								// if the job meets the deadline at t, it can meet it at t + idleTimeNeeded
								// this is because we've guaranteed that all of our tasks have adequate time between
								// release and deadline
								int idleTimeNeeded = newJob.timeBeforeRelease( t );
								int totalSlackNeeded = idleTimeNeeded + newJob.getWCET();

								if( totalSlackNeeded > this.slackAvailableDownstream( jobIndex ) )
								{
									System.err.println( "Not enough slack at index " + jobIndex + " to accomodate release time for job " + newJob.getIdentifier() + " in frame " + this.identifier );

									if( idleTimeNeeded > 0 )
										return false;
								}

								break;
							}
						}
					}

					if( jobIndex < 0 )
					{
						System.err.println( "Could not fit job " + newJob.getIdentifier() + " in frame " + this.identifier );
						return false;
					}

					this.addFrameJob( t, jobIndex, newJob );
				}
			}

			if( ! validateSchedule() )
			{
				throw new ScheduleException("Invalid frame schedule detected for frame " + this.identifier + ": " + this.getScheduleString() );
			}

			return true;
		}
		else
		{
			System.err.println( "No capacity for job " + newJob.getIdentifier() + " in frame " + this.identifier );
		}
		return false;
	}
	
	public boolean validateSchedule ( )
	{
		Iterator<FrameJob> frameJobIterator = this.jobs.iterator();
		
		int t = this.startT;
		int finishT = this.startT + this.duration;
		
		while( frameJobIterator.hasNext() )
		{
			FrameJob currFrameJob = frameJobIterator.next();
			Job taskJob = currFrameJob.getTaskJob();
			
			if( taskJob != null )
			{
				if( t < taskJob.getReleaseTime() )
				{
					System.err.println( "Warning: Job " + taskJob.getIdentifier() + " scheduled before release time [" + Integer.toString( t ) + "<" + Integer.toString( taskJob.getReleaseTime() ) + "]" );
					return false;
				}

				if( finishT < ( t + taskJob.getWCET() ) )
				{
					System.err.println( "Warning: Job " + taskJob.getIdentifier() + " finishes after frame completes [" + Integer.toString( finishT ) + "<" + Integer.toString( taskJob.getDeadline() ) + "]" );
					return false;
				}
			}
			
			t += currFrameJob.getDuration();
		}
		
		return true;
	}
	
	public String getScheduleString ()
	{
		String sched = "Schedule:";
		
		Iterator<FrameJob> frameJobIterator = this.jobs.iterator();
		
		int t = this.startT;
		int finishT = this.startT + this.duration;
		
		while( frameJobIterator.hasNext() )
		{
			FrameJob currFrameJob = frameJobIterator.next();
			int jobFinish = currFrameJob.getFinishTime();
			
			while( t < jobFinish )
			{
				sched += Frame.ScheduleEntry( t, t,  currFrameJob.getIdentifier() );
				++t;
			}
		}
		
		while( t < finishT )
		{
			sched += Frame.ScheduleEntry( t, t, "IDLE" );
			++t;
		}
		
		return sched;
	}
	
	public void printSchedule () 
	{
		System.out.println( toStringWithSchedule() );
	}
	
	public int totalCapacity ()	
	{
		return this.duration;
	}
	
	public int remainingCapacity ()
	{
		return this.duration - this.workLoad;
	}
	
	public int getStartTime()
	{
		return this.startT;
	}
	
	public int getDuration()
	{
		return this.duration;
	}
	
	public String getIdentifier()
	{
		return this.identifier;
	}
	
	public String toString ()
	{
		return "Frame|ID:" + this.identifier + "|Start:" + this.startT + "|Duration:" + this.duration + "|Work:" + this.workLoad;
	}
	
	public String toStringWithSchedule ()
	{
		return toString() + "|" + getScheduleString();
	}
	
	public static String GetNextID()
	{
		return Frame.framePrefix + Integer.toString( ++count );
	}
	
	public static void ResetID()
	{
		Frame.count = 0;
	}
	
	public static void SetPrefix ( String prefix )
	{
		Frame.framePrefix = prefix; 
	}
	
	private void addFrameJob ( int t, int jobIndex, Job newJob )
	{
		int idleTimeNeeded = newJob.timeBeforeRelease( t );
		if( idleTimeNeeded > 0 )
		{
			jobs.add( jobIndex++, new FrameJob( t, idleTimeNeeded ) );
			System.err.println( "Added idle job to frame at index " + jobIndex );
		}
		jobs.add( jobIndex, new FrameJob( t + idleTimeNeeded, newJob ) );
		System.err.println( "Added job " + newJob.getIdentifier() + " to frame at index " + jobIndex );
		this.workLoad += newJob.getWCET();
		
		consumeDownstreamSlack( ++jobIndex, ( newJob.getWCET() + idleTimeNeeded ) );
	}
	
	// recursive method to consume downstream slack	
	private void consumeDownstreamSlack ( int jobIndex, int slackDeficit )
	{
		if( jobIndex == this.jobs.size() )
			return;
		
		FrameJob checkJob = this.jobs.get( jobIndex );
		if( checkJob.isIdle() )
		{
			this.jobs.remove( jobIndex ); 
			
			int remainingIdle = checkJob.getDuration() - slackDeficit;
			
			// this one idle period will consume the entire deficit
			if( remainingIdle > 0 )
			{
				FrameJob idle = new FrameJob( checkJob.getStartTime() + slackDeficit, remainingIdle );
				this.jobs.add( jobIndex, idle );
			}
			else 
			{
				slackDeficit = 0 - remainingIdle;
			}
		}
		else
		{
			checkJob.updateStartTime( slackDeficit );
		}
		
		if( slackDeficit > 0 )
			consumeDownstreamSlack( ++jobIndex, slackDeficit );
	}
	
	// recursive method to figure out what kind of slack we have in the frame
	private int slackAvailableDownstream ( int jobIndex )
	{
		if( jobIndex == jobs.size() )
			return ( this.startT + this.duration ) - ( jobs.get( jobIndex - 1 ).getFinishTime() );
		
		int downstreamSlack = slackAvailableDownstream( jobIndex + 1 );
		
		FrameJob frameJob = jobs.get( jobIndex );
		Job taskJob =  frameJob.getTaskJob();
		if( taskJob != null )
		{
			int jobSlack = taskJob.getDeadline() - frameJob.getFinishTime();
			if( jobSlack < downstreamSlack )
				return jobSlack;
			
			return downstreamSlack;
		}

		// null task job means idle time (all slack)
		return downstreamSlack + frameJob.getDuration();
	}
	
	private static String ScheduleEntry ( int start, int end, String jobStr )
	{
		String entry;
		
		if( start == end )
			entry = Integer.toString( start );
		else
			entry = Integer.toString( start ) + "-" + Integer.toString( end );
		
		entry += "[" + jobStr + "]";
		
		return entry;
	}
	
	private LinkedList<FrameJob> jobs;
	private int duration;  	// how long does this Frame last? 
	private int workLoad; 	// how much work is scheduled in the Frame? 
	private int startT; 	// start time
	private String identifier;
	private static String framePrefix = "F";
	private static int count = 0;
}