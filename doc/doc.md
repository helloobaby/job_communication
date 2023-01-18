```
//判断System32.exe Job字段是否存在
Job = *(CurrentThread->_union_89.ApcState.Process + offsetof(_EPROCESS, Job));

if ( Job )
{
	PsGetJobServerSilo( Job, &v103 );
	v82 = v103;
}else  {//多半走这个分支
	LODWORD( v82 ) = PsGetHostSilo(); // 这个函数直接返回0的
}

// ServerSiloGlobals为全局变量PspHostSiloGlobals
ServerSiloGlobals = PsGetServerSiloGlobals(v82);
memmove((void *)lpJobObjectInformation, (const void *)ServerSiloGlobals->UserSharedData, v84);

```

```
dt _ESERVERSILO_GLOBALS &(PspHostSiloGlobals)
```

https://github.com/rbmm/MISC/blob/master/silo.cpp