from locust import HttpUser, task

class HelloWorldUser(HttpUser):

    @task
    def hello_world(self):
        self.client.post("http://vmschulz43.in.tum.de:31112/function/analyze-sentence", json={"sentence":"foo"})