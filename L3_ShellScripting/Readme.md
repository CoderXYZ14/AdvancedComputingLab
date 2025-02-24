# Commands for File Management Script

## Create all needed folders

```bash
mkdir sourcefolder
mkdir folder1
mkdir folder2
mkdir destination
```

## Create test files

```bash
echo "test" > sourcefolder/abc.txt
echo "test" > folder1/xyz.dat
echo "test" > folder2/xyz.dat
```

## Make script executable

```bash
chmod +x myp.sh
```

## Run Functionality 1 (add date to files)

```bash
./myp.sh -1 sourcefolder
```

## Run Functionality 2 (copy common files)

```bash
./myp.sh -2 folder1 folder2 destination
```
