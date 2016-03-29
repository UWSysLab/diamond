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
