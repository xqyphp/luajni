function print_service()
	--需要在 native 程调用 luajni_push_object 存储
    local app = luajni.getJavaObject("application"); --global

    local activity = luajni.callMethod(app, "getSystemService",
        "(Ljava/lang/String;)Ljava/lang/Object;", "activity")


    local services = luajni.callMethod(activity, "getRunningServices",
        "(I)Ljava/util/List;", 300)

    local size = luajni.callMethod(services, "size", "()I");


    for i = 0, size - 1 do
        local info = luajni.callMethod(services, "get", "(I)Ljava/lang/Object;", i)
        local strproc = luajni.getField(info, "process",
            "Ljava/lang/String;")
	print(strproc)
        luajni.deleteLocalRef(info)
    end
    luajni.deleteLocalRef(activity)
    luajni.deleteLocalRef(services)
end
