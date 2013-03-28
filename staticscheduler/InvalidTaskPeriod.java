package staticscheduler;

@SuppressWarnings("serial")
public class InvalidTaskPeriod extends TaskException
{
	public InvalidTaskPeriod()
	{
		super("Task period must be greater than zero.");
	}
}