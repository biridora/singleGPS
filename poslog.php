<?php
// POSTされたJSON文字列を取り出し
$postdata = file_get_contents('php://input');
$datajson = (array)json_decode($postdata, true);
$time1 = date("Ymd");
$time2 =  $datajson['times'];
$lat =  $datajson['lat'];
$lon =  $datajson['lon'];
$record= [
    [$time2,$lat,$lon]
];

$fp = fopen($time1.'.csv', 'a');

foreach ($record as $line) {
        fputcsv($fp, $line);
}

fclose($fp)
?>