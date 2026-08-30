import { HealthBar } from '../../HealthBar/HealthBar';
import './EnemyHud.css';

export const EnemyHud = ({ picture, title, hp , maxHp}) => {
  return (
    <div className="character-hud enemy">
      <div className="unit-display enemy">
        <img src={picture} className="portrait enemy" alt={title} />
        <div className="bars-container">
          <span className="bars-title enemy">{title}</span>
          <HealthBar hp={hp} maxHp={maxHp} type="sub" />
        </div>
      </div>
    </div>
  );
}