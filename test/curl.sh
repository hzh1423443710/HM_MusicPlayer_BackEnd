#!/bin/bash
# filepath: /home/hzh/test/curl.sh

# 服务器地址
HOST="localhost"
PORT="8080"

# 测试接口数组
endpoints=(
    "GET:/:首页"
    "POST:/users/register:用户注册"
    "POST:/users/login:用户登录"
    "GET:/users/123:获取用户信息"
    "PUT:/users/123:更新用户信息"
    "PUT:/users/123/password:修改密码"

    "GET:/playlists:获取歌单列表"
    "GET:/playlists/456/songs:获取歌单歌曲"
    "POST:/playlists/456/songs:添加歌曲到歌单"
    "DELETE:/playlists/456/songs/789:从歌单删除歌曲"
    "POST:/playlists:创建歌单"
    "DELETE:/playlists/456:删除歌单"

    "GET:/history:获取历史记录"
    "POST:/history:添加历史记录"
    "DELETE:/history/123:删除历史记录"
    "DELETE:/history:清空历史记录"
    "GET:/history/most_played?limit=10:获取最常播放的歌曲"
    "GET:/history/most_played?limit=10&offset=5:获取最常播放的歌曲（分页）"
)

# 单次请求测试
test_endpoint() {
    IFS=':' read -r method path description <<<"$1"
    echo "测试: $description ($method $path)"
    curl -s -H "Connection: close" -i -X "$method" "http://$HOST:$PORT$path" | head -n 1
    sleep 0.1
}

# 压力测试
stress_test() {
    local repeat=$1
    local endpoint=$2

    IFS=':' read -r method path description <<<"$endpoint"
    echo "压力测试: $description ($method $path) - $repeat 次请求"

    for ((i = 1; i <= repeat; i++)); do
        echo -n "请求 $i: "
        curl -s -H "Connection: close" -X "$method" "http://$HOST:$PORT$path" -o /dev/null -w "%{http_code}\n"
        sleep 0.01
    done
}

# 使用方式判断
if [ "$1" == "all" ]; then
    # 测试所有接口
    for endpoint in "${endpoints[@]}"; do
        test_endpoint "$endpoint"
    done
elif [ "$1" == "stress" ]; then
    # 压力测试特定接口
    stress_test "${3:-20}" "${endpoints[$2]}"
else
    # 测试特定接口指定次数
    endpoint=${endpoints[${2:-1}]}
    repeat=${1:-1}

    for ((i = 1; i <= repeat; i++)); do
        echo "第 $i 次请求:"
        IFS=':' read -r method path description <<<"$endpoint"
        curl -s -H "Connection: close" -i -X "$method" "http://$HOST:$PORT$path" | head -n 1
        echo ""
        sleep 0.1
    done
fi
