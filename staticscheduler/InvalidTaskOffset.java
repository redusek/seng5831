package staticscheduler;


@SuppressWarnings("serial")
public class InvalidTaskOffset extends TaskException
{
	public InvalidTaskOffset()
	{
		super("Task offset must be non-negative.");
	}
}