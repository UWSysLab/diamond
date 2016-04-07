[Google Cloud Registry Doc](https://cloud.google.com/container-registry/docs/)
[Kubernetes Docker Doc](http://kubernetes.io/docs/user-guide/docker-cli-to-kubectl/#docker-run)

* Log in to gcloud

```
$ gcloud auth login
```

* Get credentials for client cluster
```
$ gcloud container clusters get-credentials us-diamond --zone us-east1-b
```

* Build Image

```
$ docker build -t gcetest .
```

* tag and push the image

```
$ docker tag gcetest us.gcr.io/diamond-1239/gcetest
$ gcloud docker push us.gcr.io/diamond-1239/gcetest
```

* run the image and attach to get a terminal to the Diamond backend
```
kubectl run us-diamond -i --tty --image=us.gcr.io/diamond-1239/gcetest --restart=Never -- /home/iyzhang/run_terminalclient.py
```

* or run the benchclient
```
kubectl run us-diamond -i --tty --image=us.gcr.io/diamond-1239/gcetest --restart=Never -- /home/iyzhang/run_benchclient.py
```

* delete job after you are done
```
kubectl delete job us-diamond
```

# Alternate workflow

* Build, tag, and push image
```
$ ./build-kubernetes.pl gcetest iyzhang
```

* Run 10 instances of the scalability client
```
$ ./run-kubernetes-job.pl us-diamond gcetest run_scalability.py iyzhang 10
```

# Wrapper and convenience scripts

There are a number of scripts that make life easier. Most of these scripts are written in Perl and
perform easily understood tasks. If you ever have trouble understanding a Perl script, let Niel
know, and he can try to make a Python version.

Many of the scripts assume that there is a file called `clusters.txt` in the current working directory
with information about the Kubernetes clusters that should be used. There is a `clusters.txt` file
checked into the repo alongside all of the scripts in the `docker` folder, so you should update that
file to reflect the clusters you want to use.

Also, note that `build-kubernetes.pl` and `run-kubernetes-job.pl` have Google Cloud repository information
for the Diamond project hardcoded. Make sure you change those values if you ever use these scripts for
a different project.

* build-kubernetes.pl
Builds a Docker image using the Dockerfile in the working directory, tags it, and pushes it to the Google Cloud
repository.

* run-kubernetes-job.pl
Starts multiple instances of a Docker image running a Python script from the Dockerfile, spread over multiple Kubernetes
clusters.

* kubectl-get-pods.pl
A wrapper around the `kubectl get pods` command that executes it for every cluster.

* kubectl-logs.pl
A wrapper around the `kubectl logs` command that takes care of switching to the right cluster.

* kubectl-delete-job.pl
A wrapper around the `kubectl delete job` command that deletes the job on every cluster.