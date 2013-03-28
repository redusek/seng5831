package staticscheduler;

@SuppressWarnings("serial")
public class InvalidTaskPriority extends TaskException
{
	public InvalidTaskPriority()
	{
		super("Task priority must be greather than zero.");
	}
}