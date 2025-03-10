<?php
namespace App\Model\Table;
	
use Cake\ORM\Table;
	
class EntertainareaTable extends Table{
	
public function initialize(array $config){
	$this->setTable('EntertainArea');
	$this->primaryKey('PK_EntertainArea');	
	
		$this->belongsTo('Room',
				  [
					  'className' => 'Rooms',					  
					  'foreignKey' => 'FK_Room',
					  'bindingKey' => 'PK_Room'
				  ]);
	
		$this->belongsToMany('CommandGroup',				  [									  
					  'className' => 'Commandgroup',
					  'joinTable' => 'Commandgroupentertainarea',
					  'bindingKey' => 'PK_EntertainArea',
					  'foreignKey' => 'FK_EntertainArea',
					  'targetForeignKey' => 'FK_CommandGroup'
				  ]);	
		
}
	
		
 function getEntertainAreaFromName($name){
	$targetEa =	$this->find('all')
			->where(['Description LIKE' => $name])
		->toList();
		
	if( isset($targetEa) ){
		return $targetEa[0]['PK_EntertainArea'];
	}	
	return "-1";
}
	
}



?>