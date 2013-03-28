package staticscheduler;

public class StaticScheduler 
{
	public static void main(String[] args) 
	{
		Hyperperiod executionScope = new Hyperperiod();
		
		Task.SetPrefix("A");
		System.out.println("Scheduling workload 1 as task set A");
		
		// workload 1
		try 
		{
			int priority = 0;
			executionScope.addTask( new Task( 10, 2, 0, 10, ++priority ) );
			executionScope.addTask( new Task( 10, 2, 0, 10, ++priority ) );
			executionScope.addTask( new Task( 20, 1, 5, 20, ++priority ) );
			executionScope.addTask( new Task( 20, 2, 5, 20, ++priority ) );
			executionScope.addTask( new Task( 40, 2, 5, 30, ++priority ) );
			executionScope.addTask( new Task( 40, 2, 5, 30, ++priority ) );
			executionScope.addTask( new Task( 80, 2, 10, 60, ++priority ) );
			executionScope.addTask( new Task( 80, 2, 10, 60, ++priority ) );
			
			executionScope.printTasks();
			executionScope.printSchedule();
		}
		catch( TaskException e )
		{
			System.out.println("Task Exception: " + e.getMessage());
		}
		catch( ScheduleException e)
		{
			System.out.println("Schedule Exception: " + e.getMessage());
		}
		
		Task.ResetID();
		
		executionScope.clear();
		
		Task.SetPrefix("B");
		System.out.println("Scheduling workload 2 as task set B");
		
		// workload 2
		try 
		{
			int priority = 0;
			executionScope.addTask( new Task( 20, 4, 0, 15, ++priority ) );
			executionScope.addTask( new Task( 20, 1, 5, 20, ++priority ) );
			executionScope.addTask( new Task( 30, 2, 5, 30, ++priority ) );
			executionScope.addTask( new Task( 30, 1, 5, 30, ++priority ) );
			executionScope.addTask( new Task( 50, 1, 10, 40, ++priority ) );
			executionScope.addTask( new Task( 50, 1, 10, 40, ++priority ) );
			executionScope.addTask( new Task( 50, 2, 25, 50, ++priority ) );
			executionScope.addTask( new Task( 50, 5, 25, 50, ++priority ) );
			
			executionScope.printTasks();
			executionScope.printSchedule();
		}
		catch( TaskException e )
		{
			System.out.println("Exception: " + e.getMessage());
		}
		catch( ScheduleException e)
		{
			System.out.println("Schedule Exception: " + e.getMessage());
		}
		
		Task.ResetID();
		
	}
}
