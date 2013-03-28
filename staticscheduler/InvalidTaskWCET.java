package staticscheduler;

@SuppressWarnings("serial")
public class InvalidTaskWCET extends TaskException
{
	public InvalidTaskWCET()
	{
		super("Task WCET must be greater than zero.");
	}
}